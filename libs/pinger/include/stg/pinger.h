 /*
 $Revision: 1.8 $
 $Date: 2008/05/10 11:59:53 $
 $Author: nobunaga $
 */

#pragma once

#include <string>
#include <list>
#include <map>
#include <ctime>
#include <cstdint>

#ifdef LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#endif

#if defined (FREE_BSD) || defined(DARWIN)
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#include <arpa/inet.h>
#endif

//-----------------------------------------------------------------------------
struct ICMP_HDR
{
    uint8_t       type;
    uint8_t       code;
    uint16_t      checksum;
    union
    {
        struct
        {
            uint16_t    id;
            uint16_t    sequence;
        } echo;
        uint32_t  gateway;
        struct
        {
            uint16_t    unused;
            uint16_t    mtu;
        } frag;
    } un;
};
//-----------------------------------------------------------------------------
struct IP_HDR
{
    uint8_t     ihl:4,
                version:4;
    uint8_t     tos;
    uint16_t    tot_len;
    uint16_t    id;
    uint16_t    frag_off;
    uint8_t     ttl;
    uint8_t     protocol;
    uint16_t    check;
    uint32_t    saddr;
    uint32_t    daddr;
};
//-----------------------------------------------------------------------------
struct PING_IP_TIME
{
    uint32_t    ip;
    time_t      pingTime;
};
//-----------------------------------------------------------------------------

#define PING_DATA_LEN   (64)
//-----------------------------------------------------------------------------
struct PING_MESSAGE
{
    ICMP_HDR    hdr;
    char        msg[PING_DATA_LEN];
};
//-----------------------------------------------------------------------------
class STG_PINGER
{
    public:
        typedef std::multimap<uint32_t, time_t> PingIPs;
        typedef PingIPs::size_type SizeType;

                explicit STG_PINGER(time_t delay = 15);
                ~STG_PINGER();

        int     Start();
        int     Stop();
        void    AddIP(uint32_t ip);
        void    DelIP(uint32_t ip);
        SizeType GetPingIPNum() const { return m_pingIP.size(); }
        void    PrintAllIP();
        int     GetIPTime(uint32_t ip, time_t * t) const;
        void    SetDelayTime(time_t d) { m_delay = d; }
        time_t  GetDelayTime() const { return m_delay; }
        const std::string & GetStrError() const { return m_errorStr; }

    private:
        uint16_t    PingCheckSum(void * data, int len);
        int         SendPing(uint32_t ip);
        uint32_t    RecvPing();
        void        RealAddIP();
        void        RealDelIP();

        static void * RunSendPing(void * d);
        static void * RunRecvPing(void * d);

        time_t      m_delay;
        bool        m_nonstop;
        bool        m_isRunningRecver;
        bool        m_isRunningSender;
        int         m_sendSocket;
        int         m_recvSocket;
        pthread_t   m_sendThread;
        pthread_t   m_recvThread;

        PING_MESSAGE m_pmSend;
        uint32_t    m_pid;

        std::string m_errorStr;

        std::multimap<uint32_t, time_t> m_pingIP;
        std::list<uint32_t>          m_ipToAdd;
        std::list<uint32_t>          m_ipToDel;

        mutable pthread_mutex_t m_mutex;
};
