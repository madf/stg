#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>

#include "pinger.h"
#include "stg/common.h"
#include "stg/stg_locker.h"

#ifdef STG_TIME
extern volatile time_t stgTime;
#endif

//-----------------------------------------------------------------------------
STG_PINGER::STG_PINGER(time_t d)
    : delay(d),
      nonstop(false),
      isRunningRecver(false),
      isRunningSender(false),
      sendSocket(-1),
      recvSocket(-1),
      pid(0)
{
    pthread_mutex_init(&mutex, NULL);
    memset(&pmSend, 0, sizeof(pmSend));
}
//-----------------------------------------------------------------------------
STG_PINGER::~STG_PINGER()
{
    pthread_mutex_destroy(&mutex);
}
//-----------------------------------------------------------------------------
int STG_PINGER::Start()
{
    struct protoent *proto = NULL;
    proto = getprotobyname("ICMP");
    sendSocket = socket(PF_INET, SOCK_RAW, proto->p_proto);
    recvSocket = socket(PF_INET, SOCK_RAW, proto->p_proto);
    nonstop = true;
    pid = (int) getpid() % 65535;
    if (sendSocket < 0 || recvSocket < 0)
        {
        errorStr = "Cannot create socket.";
        return -1;
        }

    if (pthread_create(&sendThread, NULL, RunSendPing, this))
        {
        errorStr = "Cannot create send thread.";
        return -1;
        }

    if (pthread_create(&recvThread, NULL, RunRecvPing, this))
        {
        errorStr = "Cannot create recv thread.";
        return -1;
        }

    return 0;
}
//-----------------------------------------------------------------------------
int STG_PINGER::Stop()
{
    close(recvSocket);
    nonstop = false;
    if (isRunningRecver)
        {
        //5 seconds to thread stops itself
        for (size_t i = 0; i < 25; i++)
            {
            if (i % 5 == 0)
                SendPing(0x0100007f);//127.0.0.1

            if (!isRunningRecver)
                break;

            usleep(200000);
            }

        //after 5 seconds waiting thread still running. now killing it
        if (isRunningRecver)
            {
            if (pthread_kill(recvThread, SIGINT))
                {
                errorStr = "Cannot kill thread.";
                return -1;
                }
            }
        }

    if (isRunningSender)
        {
        //5 seconds to thread stops itself
        for (size_t i = 0; i < 25; i++)
            {
            if (!isRunningSender)
                break;

            usleep(200000);
            }

        //after 5 seconds waiting thread still running. now killing it
        if (isRunningSender)
            {
            if (pthread_kill(sendThread, SIGINT))
                {
                errorStr = "Cannot kill thread.";
                return -1;
                }
            }
        }

    close(sendSocket);
    return 0;
}
//-----------------------------------------------------------------------------
void STG_PINGER::AddIP(uint32_t ip)
{
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    ipToAdd.push_back(ip);
}
//-----------------------------------------------------------------------------
void STG_PINGER::DelIP(uint32_t ip)
{
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    ipToDel.push_back(ip);
}
//-----------------------------------------------------------------------------
void STG_PINGER::RealAddIP()
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);

    std::list<uint32_t>::iterator iter;
    iter = ipToAdd.begin();
    while (iter != ipToAdd.end())
        {
        pingIP.insert(std::make_pair(*iter, 0));
        ++iter;
        }
    ipToAdd.erase(ipToAdd.begin(), ipToAdd.end());
    }
//-----------------------------------------------------------------------------
void STG_PINGER::RealDelIP()
{
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);

    std::list<uint32_t>::iterator iter;
    std::multimap<uint32_t, time_t>::iterator treeIter;
    iter = ipToDel.begin();
    while (iter != ipToDel.end())
        {
        treeIter = pingIP.find(*iter);
        if (treeIter != pingIP.end())
            pingIP.erase(treeIter);

        ++iter;
        }
    ipToDel.erase(ipToDel.begin(), ipToDel.end());
}
//-----------------------------------------------------------------------------
int STG_PINGER::GetPingIPNum() const
{
    return pingIP.size();
}
//-----------------------------------------------------------------------------
void STG_PINGER::PrintAllIP()
{
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    std::multimap<uint32_t, time_t>::iterator iter;
    iter = pingIP.begin();
    while (iter != pingIP.end())
        {
        uint32_t ip = iter->first;
        time_t t = iter->second;
        std::string s;
        x2str(t, s);
        printf("ip = %s, time = %9s\n", inet_ntostring(ip).c_str(), s.c_str());
        ++iter;
        }

}
//-----------------------------------------------------------------------------
int STG_PINGER::GetIPTime(uint32_t ip, time_t * t) const
{
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    std::multimap<uint32_t, time_t>::const_iterator treeIter;

    treeIter = pingIP.find(ip);
    if (treeIter == pingIP.end())
        return -1;

    *t = treeIter->second;
    return 0;
}
//-----------------------------------------------------------------------------
uint16_t STG_PINGER::PingCheckSum(void * data, int len)
{
    unsigned short * buf = (unsigned short *)data;
    unsigned int sum = 0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;

    if ( len == 1 )
        sum += *(unsigned char*)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}
//-----------------------------------------------------------------------------
int STG_PINGER::SendPing(uint32_t ip)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = ip;

    memset(&pmSend, 0, sizeof(pmSend));
    pmSend.hdr.type = ICMP_ECHO;
    pmSend.hdr.un.echo.id = pid;
    memcpy(pmSend.msg, &ip, sizeof(ip));

    pmSend.hdr.checksum = PingCheckSum(&pmSend, sizeof(pmSend));

    if (sendto(sendSocket, &pmSend, sizeof(pmSend), 0, (sockaddr *)&addr, sizeof(addr)) <= 0 )
        {
        errorStr = "Send ping error: " + std::string(strerror(errno));
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
    int bytes;
    socklen_t len = sizeof(addr);

    bytes = recvfrom(recvSocket, &buf, sizeof(buf), 0, (struct sockaddr*)&addr, &len);
    if (bytes > 0)
        {
        struct IP_HDR * ip = (struct IP_HDR *)buf;
        struct ICMP_HDR *icmp = (struct ICMP_HDR *)(buf + ip->ihl * 4);

        if (icmp->un.echo.id != pid)
            return 0;

        ipAddr = *(uint32_t*)(buf + sizeof(ICMP_HDR) + ip->ihl * 4);
        }

    return ipAddr;
}
//-----------------------------------------------------------------------------
void * STG_PINGER::RunSendPing(void * d)
{
    STG_PINGER * pinger = static_cast<STG_PINGER *>(d);

    pinger->isRunningSender = true;
    time_t lastPing = 0;
    while (pinger->nonstop)
        {
        pinger->RealAddIP();
        pinger->RealDelIP();

        std::multimap<uint32_t, time_t>::iterator iter;
        iter = pinger->pingIP.begin();
        while (iter != pinger->pingIP.end())
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

        while (currTime - lastPing < pinger->delay && pinger->nonstop)
            {
            #ifdef STG_TIME
            currTime = stgTime;
            #else
            currTime = time(NULL);
            #endif
            usleep(20000);
            }
        }

    pinger->isRunningSender = false;

    return NULL;
}
//-----------------------------------------------------------------------------
void * STG_PINGER::RunRecvPing(void * d)
{
    STG_PINGER * pinger = static_cast<STG_PINGER *>(d);

    pinger->isRunningRecver = true;

    while (pinger->nonstop)
        {
        uint32_t ip = pinger->RecvPing();

        if (ip)
            {
            std::multimap<uint32_t, time_t>::iterator treeIterUpper = pinger->pingIP.upper_bound(ip);
            std::multimap<uint32_t, time_t>::iterator treeIterLower = pinger->pingIP.lower_bound(ip);
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
    pinger->isRunningRecver = false;
    return NULL;
}
//-----------------------------------------------------------------------------
