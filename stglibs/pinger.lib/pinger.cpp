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
#include "common.h"
#include "stg_locker.h"

#ifdef STG_TIME
extern volatile time_t stgTime;
#endif

//-----------------------------------------------------------------------------
STG_PINGER::STG_PINGER(time_t d)
{
    delay = d;
    pthread_mutex_init(&mutex, NULL);
    pid = 0;
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
        int i;
        for (i = 0; i < 25; i++)
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
            //if (pthread_kill(recvThread, SIGINT))
            //    {
                errorStr = "Cannot kill thread.";
                return -1;
            //    }
            //printf("recvThread killed\n");
            }
        }

    if (isRunningSender)
        {
        //5 seconds to thread stops itself
        int i;
        for (i = 0; i < 25; i++)
            {
            if (!isRunningSender)
                break;

            usleep(200000);
            }

        //after 5 seconds waiting thread still running. now killing it
        if (isRunningSender)
            {
            //if (pthread_kill(sendThread, SIGINT))
            //    {
                errorStr = "Cannot kill thread.";
                return -1;
            //    }
            //printf("sendThread killed\n");
            }
        }

    close(sendSocket);
    return 0;
}
//-----------------------------------------------------------------------------
void STG_PINGER::AddIP(uint32_t ip)
{
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    //printf("AddIP\n");
    ipToAdd.push_back(ip);
}
//-----------------------------------------------------------------------------
void STG_PINGER::DelIP(uint32_t ip)
{
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    //printf("DelIP\n");
    ipToDel.push_back(ip);
}
//-----------------------------------------------------------------------------
void STG_PINGER::RealAddIP()
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);

    list<uint32_t>::iterator iter;
    iter = ipToAdd.begin();
    while (iter != ipToAdd.end())
        {
        /*packets.insert(pair<RAW_PACKET, PACKET_EXTRA_DATA>(rawPacket, ed));*/
        //pingIP[*iter] = 0;
        pingIP.insert(pair<uint32_t, time_t>(*iter, 0));
        iter++;
        }
    ipToAdd.erase(ipToAdd.begin(), ipToAdd.end());
    }
//-----------------------------------------------------------------------------
void STG_PINGER::RealDelIP()
{
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);

    list<uint32_t>::iterator iter;
    multimap<uint32_t, time_t>::iterator treeIter;
    iter = ipToDel.begin();
    while (iter != ipToDel.end())
        {
        treeIter = pingIP.find(*iter);
        //printf("Found %X\n", *iter);
        if (treeIter != pingIP.end())
            pingIP.erase(treeIter);

        iter++;
        }
    ipToDel.erase(ipToDel.begin(), ipToDel.end());
}
//-----------------------------------------------------------------------------
int STG_PINGER::GetPingIPNum()
{
    return pingIP.size();
}
//-----------------------------------------------------------------------------
void STG_PINGER::GetAllIP(vector<PING_IP_TIME> *)
{
    //STG_LOCKER lock(&mutex, __FILE__, __LINE__);
}
//-----------------------------------------------------------------------------
void STG_PINGER::PrintAllIP()
{
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    multimap<uint32_t, time_t>::iterator iter;
    iter = pingIP.begin();
    while (iter != pingIP.end())
        {
        uint32_t ip = iter->first;
        time_t t = iter->second;
        string s;
        x2str(t, s);
        printf("ip = %s, time = %9s\n", inet_ntostring(ip).c_str(), s.c_str());
        iter++;
        }

}
//-----------------------------------------------------------------------------
int STG_PINGER::GetIPTime(uint32_t ip, time_t * t)
{
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    multimap<uint32_t, time_t>::iterator treeIter;

    treeIter = pingIP.find(ip);
    if (treeIter == pingIP.end())
        return -1;

    *t = treeIter->second;
    return 0;
}
//-----------------------------------------------------------------------------
void STG_PINGER::SetDelayTime(time_t d)
{
    delay = d;
}
//-----------------------------------------------------------------------------
time_t STG_PINGER::GetDelayTime()
{
    return delay;
}
//-----------------------------------------------------------------------------
string STG_PINGER::GetStrError()
{
    return errorStr;
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
    //printf("SendPing %X \n", ip);
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
        errorStr = "Send ping error: " + string(strerror(errno));
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
    //printf("recvfrom\n");
    if (bytes > 0)
        {
        struct IP_HDR * ip = (struct IP_HDR *)buf;
        struct ICMP_HDR *icmp = (struct ICMP_HDR *)(buf + ip->ihl * 4);

        //printf("icmp->un.echo.id=%d,  pid=%d, tid: %d\n", icmp->un.echo.id, pid);
        if (icmp->un.echo.id != pid)
            return 0;

        ipAddr = *(uint32_t*)(buf + sizeof(ICMP_HDR) + ip->ihl * 4);
        }

    return ipAddr;
}
//-----------------------------------------------------------------------------
void * STG_PINGER::RunSendPing(void * d)
{
    STG_PINGER * pinger = (STG_PINGER*)d;

    pinger->isRunningSender = true;
    time_t lastPing = 0;
    while (pinger->nonstop)
        {
        pinger->RealAddIP();
        pinger->RealDelIP();

        multimap<uint32_t, time_t>::iterator iter;
        iter = pinger->pingIP.begin();
        while (iter != pinger->pingIP.end())
            {
            uint32_t ip = iter->first;
            pinger->SendPing(ip);
            iter++;
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
        //printf("new ping cycle\n");
        }

    pinger->isRunningSender = false;

    return NULL;
}
//-----------------------------------------------------------------------------
void * STG_PINGER::RunRecvPing(void * d)
{
    STG_PINGER * pinger = (STG_PINGER*)d;

    pinger->isRunningRecver = true;

    uint32_t ip;
    multimap<uint32_t, time_t>::iterator treeIterLower;
    multimap<uint32_t, time_t>::iterator treeIterUpper;

    while (pinger->nonstop)
        {
        ip = pinger->RecvPing();

        if (ip)
            {
            //printf("RecvPing %X\n", ip);
            treeIterUpper = pinger->pingIP.upper_bound(ip);
            treeIterLower = pinger->pingIP.lower_bound(ip);
            int i = 0;
            while (treeIterUpper != treeIterLower)
            //treeIterUpper = pinger->pingIP.find(ip);
            //if (treeIterUpper != pinger->pingIP.end())
                {
                //printf("+++! time=%d %X i=%d !+++\n", time(NULL), ip, i);
                //printf("--- time=%d ---\n", time(NULL));
                #ifdef STG_TIME
                treeIterLower->second = stgTime;
                #else
                treeIterLower->second = time(NULL);
                #endif
                ++treeIterLower;
                i++;
                }
            }

        }
    pinger->isRunningRecver = false;
    return NULL;
}
//-----------------------------------------------------------------------------

