#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#include <cstdlib>
#include <csignal>
#include <cstring>
#include <cerrno>
#include <cmath>
#include <cstdio>

#include "stg/common.h"
#include "stg/locker.h"

#include "stg/pinger.h"

#ifdef STG_TIME
extern volatile time_t stgTime;
#endif

//-----------------------------------------------------------------------------
STG_PINGER::STG_PINGER(time_t d)
    : m_delay(d),
      m_nonstop(false),
      m_isRunningRecver(false),
      m_isRunningSender(false),
      m_sendSocket(-1),
      m_recvSocket(-1),
      m_pid(0)
{
pthread_mutex_init(&m_mutex, NULL);
memset(&m_pmSend, 0, sizeof(m_pmSend));
}
//-----------------------------------------------------------------------------
STG_PINGER::~STG_PINGER()
{
pthread_mutex_destroy(&m_mutex);
}
//-----------------------------------------------------------------------------
int STG_PINGER::Start()
{
struct protoent *proto = NULL;
proto = getprotobyname("ICMP");
m_sendSocket = socket(PF_INET, SOCK_RAW, proto->p_proto);
m_recvSocket = socket(PF_INET, SOCK_RAW, proto->p_proto);
m_nonstop = true;
m_pid = static_cast<uint32_t>(getpid()) % 65535;
if (m_sendSocket < 0 || m_recvSocket < 0)
    {
    m_errorStr = "Cannot create socket.";
    return -1;
    }

if (pthread_create(&m_sendThread, NULL, RunSendPing, this))
    {
    m_errorStr = "Cannot create send thread.";
    return -1;
    }

if (pthread_create(&m_recvThread, NULL, RunRecvPing, this))
    {
    m_errorStr = "Cannot create recv thread.";
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int STG_PINGER::Stop()
{
close(m_recvSocket);
m_nonstop = false;
if (m_isRunningRecver)
    {
    //5 seconds to thread stops itself
    for (size_t i = 0; i < 25; i++)
        {
        if (i % 5 == 0)
            SendPing(0x0100007f);//127.0.0.1

        if (!m_isRunningRecver)
            break;

        struct timespec ts = {0, 200000000};
        nanosleep(&ts, NULL);
        }
    }

if (m_isRunningSender)
    {
    //5 seconds to thread stops itself
    for (size_t i = 0; i < 25; i++)
        {
        if (!m_isRunningSender)
            break;

        struct timespec ts = {0, 200000000};
        nanosleep(&ts, NULL);
        }
    }

close(m_sendSocket);

if (m_isRunningSender || m_isRunningRecver)
    return -1;

return 0;
}
//-----------------------------------------------------------------------------
void STG_PINGER::AddIP(uint32_t ip)
{
STG_LOCKER lock(&m_mutex);
m_ipToAdd.push_back(ip);
}
//-----------------------------------------------------------------------------
void STG_PINGER::DelIP(uint32_t ip)
{
STG_LOCKER lock(&m_mutex);
m_ipToDel.push_back(ip);
}
//-----------------------------------------------------------------------------
void STG_PINGER::RealAddIP()
{
STG_LOCKER lock(&m_mutex);

auto iter = m_ipToAdd.begin();
while (iter != m_ipToAdd.end())
    {
    m_pingIP.insert(std::make_pair(*iter, 0));
    ++iter;
    }
m_ipToAdd.erase(m_ipToAdd.begin(), m_ipToAdd.end());
}
//-----------------------------------------------------------------------------
void STG_PINGER::RealDelIP()
{
STG_LOCKER lock(&m_mutex);

auto iter = m_ipToDel.begin();
while (iter != m_ipToDel.end())
    {
    auto treeIter = m_pingIP.find(*iter);
    if (treeIter != m_pingIP.end())
        m_pingIP.erase(treeIter);

    ++iter;
    }
m_ipToDel.erase(m_ipToDel.begin(), m_ipToDel.end());
}
//-----------------------------------------------------------------------------
void STG_PINGER::PrintAllIP()
{
STG_LOCKER lock(&m_mutex);
auto iter = m_pingIP.begin();
while (iter != m_pingIP.end())
    {
    uint32_t ip = iter->first;
    time_t t = iter->second;
    std::string s = std::to_string(t);
    printf("ip = %s, time = %9s\n", inet_ntostring(ip).c_str(), s.c_str());
    ++iter;
    }

}
//-----------------------------------------------------------------------------
int STG_PINGER::GetIPTime(uint32_t ip, time_t * t) const
{
STG_LOCKER lock(&m_mutex);

auto treeIter = m_pingIP.find(ip);
if (treeIter == m_pingIP.end())
    return -1;

*t = treeIter->second;
return 0;
}
//-----------------------------------------------------------------------------
uint16_t STG_PINGER::PingCheckSum(void * data, int len)
{
uint16_t * buf = static_cast<uint16_t *>(data);
uint32_t sum = 0;
uint32_t result;

for ( sum = 0; len > 1; len -= 2 )
    sum += *buf++;

if ( len == 1 )
    sum += *reinterpret_cast<uint8_t*>(buf);

sum = (sum >> 16) + (sum & 0xFFFF);
sum += (sum >> 16);
result = ~sum;
return static_cast<uint16_t>(result);
}
//-----------------------------------------------------------------------------
int STG_PINGER::SendPing(uint32_t ip)
{
struct sockaddr_in addr;
memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_port = 0;
addr.sin_addr.s_addr = ip;

memset(&m_pmSend, 0, sizeof(m_pmSend));
m_pmSend.hdr.type = ICMP_ECHO;
m_pmSend.hdr.un.echo.id = static_cast<uint16_t>(m_pid);
memcpy(m_pmSend.msg, &ip, sizeof(ip));

m_pmSend.hdr.checksum = PingCheckSum(&m_pmSend, sizeof(m_pmSend));

if (sendto(m_sendSocket, &m_pmSend, sizeof(m_pmSend), 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) <= 0 )
    {
    m_errorStr = "Send ping error: " + std::string(strerror(errno));
    return -1;
    }


return 0;
}
//-----------------------------------------------------------------------------
uint32_t STG_PINGER::RecvPing()
{
struct sockaddr_in addr;
uint32_t ipAddr = 0;

char buf[128];
memset(buf, 0, sizeof(buf));
socklen_t len = sizeof(addr);

if (recvfrom(m_recvSocket, &buf, sizeof(buf), 0, reinterpret_cast<struct sockaddr*>(&addr), &len))
    {
    struct IP_HDR * ip = static_cast<struct IP_HDR *>(static_cast<void *>(buf));
    struct ICMP_HDR *icmp = static_cast<struct ICMP_HDR *>(static_cast<void *>(buf + ip->ihl * 4));

    if (icmp->un.echo.id != m_pid)
        return 0;

    ipAddr = *static_cast<uint32_t*>(static_cast<void *>(buf + sizeof(ICMP_HDR) + ip->ihl * 4));
    }

return ipAddr;
}
//-----------------------------------------------------------------------------
void * STG_PINGER::RunSendPing(void * d)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

auto* pinger = static_cast<STG_PINGER *>(d);

pinger->m_isRunningSender = true;
time_t lastPing = 0;
while (pinger->m_nonstop)
    {
    pinger->RealAddIP();
    pinger->RealDelIP();

    std::multimap<uint32_t, time_t>::iterator iter;
    iter = pinger->m_pingIP.begin();
    while (iter != pinger->m_pingIP.end())
        {
        pinger->SendPing(iter->first);
        ++iter;
        }

    time_t currTime;

    #ifdef STG_TIME
    lastPing = stgTime;
    currTime = stgTime;
    #else
    currTime = lastPing = time(NULL);
    #endif

    while (currTime - lastPing < pinger->m_delay && pinger->m_nonstop)
        {
        #ifdef STG_TIME
        currTime = stgTime;
        #else
        currTime = time(NULL);
        #endif
        struct timespec ts = {0, 20000000};
        nanosleep(&ts, NULL);
        }
    }

pinger->m_isRunningSender = false;

return NULL;
}
//-----------------------------------------------------------------------------
void * STG_PINGER::RunRecvPing(void * d)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

auto* pinger = static_cast<STG_PINGER *>(d);

pinger->m_isRunningRecver = true;

while (pinger->m_nonstop)
    {
    uint32_t ip = pinger->RecvPing();

    if (ip)
        {
        auto treeIterUpper = pinger->m_pingIP.upper_bound(ip);
        auto treeIterLower = pinger->m_pingIP.lower_bound(ip);
        while (treeIterUpper != treeIterLower)
            {
            #ifdef STG_TIME
            treeIterLower->second = stgTime;
            #else
            treeIterLower->second = time(NULL);
            #endif
            ++treeIterLower;
            }
        }

    }
pinger->m_isRunningRecver = false;
return NULL;
}
//-----------------------------------------------------------------------------
