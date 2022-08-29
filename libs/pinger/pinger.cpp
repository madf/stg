#include "stg/pinger.h"

#include "stg/common.h"

#include <chrono>
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <cerrno>
#include <cmath>
#include <cstdio>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef STG_TIME
extern volatile time_t stgTime;
#endif

//-----------------------------------------------------------------------------
STG_PINGER::STG_PINGER(unsigned d)
    : m_delay(d),
      m_isRunningRecver(false),
      m_isRunningSender(false),
      m_sendSocket(-1),
      m_recvSocket(-1),
      m_pid(0)
{
    memset(&m_pmSend, 0, sizeof(m_pmSend));
}
//-----------------------------------------------------------------------------
bool STG_PINGER::Start()
{
    auto* proto = getprotobyname("ICMP");
    m_sendSocket = socket(PF_INET, SOCK_RAW, proto->p_proto);
    m_recvSocket = socket(PF_INET, SOCK_RAW, proto->p_proto);
    m_pid = static_cast<uint32_t>(getpid()) % 65535;
    if (m_sendSocket < 0 || m_recvSocket < 0)
    {
        m_errorStr = "Cannot create socket.";
        return false;
    }

    m_sendThread = std::jthread([this](auto token){ RunSendPing(std::move(token)); });
    m_recvThread = std::jthread([this](auto token){ RunRecvPing(std::move(token)); });

    return true;
}
//-----------------------------------------------------------------------------
bool STG_PINGER::Stop()
{
    close(m_recvSocket);
    m_sendThread.request_stop();
    if (m_isRunningRecver)
    {
        //5 seconds to thread stops itself
        for (size_t i = 0; i < 25; i++)
        {
            if (i % 5 == 0)
                SendPing(0x0100007f);//127.0.0.1

            if (!m_isRunningRecver)
                break;

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    if (m_isRunningSender)
    {
        //5 seconds to thread stops itself
        for (size_t i = 0; i < 25; i++)
        {
            if (!m_isRunningSender)
                break;

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    close(m_sendSocket);

    return !m_isRunningSender && !m_isRunningRecver;
}
//-----------------------------------------------------------------------------
void STG_PINGER::AddIP(uint32_t ip)
{
    std::lock_guard lock(m_mutex);
    m_pingIPs.insert(std::make_pair(ip, 0));
}
//-----------------------------------------------------------------------------
void STG_PINGER::DelIP(uint32_t ip)
{
    std::lock_guard lock(m_mutex);
    auto it = m_pingIPs.find(ip);
    if (it != m_pingIPs.end())
        m_pingIPs.erase(it);
}
//-----------------------------------------------------------------------------
void STG_PINGER::PrintAllIP()
{
    std::lock_guard lock(m_mutex);
    for (auto kv : m_pingIPs)
    {
        uint32_t ip = kv.first;
        time_t t = kv.second;
        std::string s = std::to_string(t);
        printf("ip = %s, time = %9s\n", inet_ntostring(ip).c_str(), s.c_str());
    }
}
//-----------------------------------------------------------------------------
bool STG_PINGER::GetIPTime(uint32_t ip, time_t& t) const
{
    std::lock_guard lock(m_mutex);

    auto it = m_pingIPs.find(ip);
    if (it == m_pingIPs.end())
        return false;

    t = it->second;
    return true;
}
//-----------------------------------------------------------------------------
uint16_t STG_PINGER::PingCheckSum(const void* data, int len)
{
    const auto* buf = static_cast<const uint16_t*>(data);
    uint32_t sum = 0;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;

    if ( len == 1 )
        sum += *reinterpret_cast<const uint8_t*>(buf);

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    return ~sum;
}
//-----------------------------------------------------------------------------
bool STG_PINGER::SendPing(uint32_t ip)
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
        return false;
    }


    return true;
}
//-----------------------------------------------------------------------------
uint32_t STG_PINGER::RecvPing()
{
    struct sockaddr_in addr;
    uint32_t ipAddr = 0;

    uint8_t buf[128];
    memset(buf, 0, sizeof(buf));
    socklen_t len = sizeof(addr);

    if (recvfrom(m_recvSocket, &buf, sizeof(buf), 0, reinterpret_cast<struct sockaddr*>(&addr), &len))
    {
        auto* ip = static_cast<struct IP_HDR *>(static_cast<void *>(buf));
        auto* icmp = static_cast<struct ICMP_HDR *>(static_cast<void *>(buf + ip->ihl * 4));

        if (icmp->un.echo.id != m_pid)
            return 0;

        ipAddr = *static_cast<uint32_t*>(static_cast<void *>(buf + sizeof(ICMP_HDR) + ip->ihl * 4));
    }

    return ipAddr;
}
//-----------------------------------------------------------------------------
void STG_PINGER::RunSendPing(std::stop_token token)
{
    sigset_t signalSet;
    sigfillset(&signalSet);
    pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

    m_isRunningSender = true;
    time_t lastPing = 0;
    while (!token.stop_requested())
    {
        PingIPs ips;
        {
            std::lock_guard lock(m_mutex);
            ips = m_pingIPs;
        }

        for (const auto& kv : ips)
            SendPing(kv.first);

        time_t currTime;

        #ifdef STG_TIME
        lastPing = stgTime;
        currTime = stgTime;
        #else
        currTime = lastPing = time(NULL);
        #endif

        while (currTime - lastPing < m_delay && !token.stop_requested())
        {
            #ifdef STG_TIME
            currTime = stgTime;
            #else
            currTime = time(NULL);
            #endif
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    m_isRunningSender = false;
}
//-----------------------------------------------------------------------------
void STG_PINGER::RunRecvPing(std::stop_token token)
{
    sigset_t signalSet;
    sigfillset(&signalSet);
    pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

    m_isRunningRecver = true;

    while (!token.stop_requested())
    {
        uint32_t ip = RecvPing();

        if (ip)
        {
            std::lock_guard lock(m_mutex);
            auto treeIterUpper = m_pingIPs.upper_bound(ip);
            auto treeIterLower = m_pingIPs.lower_bound(ip);
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
    m_isRunningRecver = false;
}
//-----------------------------------------------------------------------------
