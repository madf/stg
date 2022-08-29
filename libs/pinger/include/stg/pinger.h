 /*
 $Revision: 1.8 $
 $Date: 2008/05/10 11:59:53 $
 $Author: nobunaga $
 */

#pragma once

#include <string>
#include <map>
#include <mutex>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop
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
        using PingIPs = std::multimap<uint32_t, time_t>;

                explicit STG_PINGER(unsigned delay = 15);

        bool    Start();
        bool    Stop();
        void    AddIP(uint32_t ip);
        void    DelIP(uint32_t ip);
        auto    GetPingIPNum() const { std::lock_guard lock(m_mutex); return m_pingIPs.size(); }
        void    PrintAllIP();
        bool    GetIPTime(uint32_t ip, time_t& t) const;
        void    SetDelayTime(unsigned d) { m_delay = d; }
        unsigned GetDelayTime() const { return m_delay; }
        const std::string& GetStrError() const { return m_errorStr; }

    private:
        uint16_t    PingCheckSum(const void* data, int len);
        bool        SendPing(uint32_t ip);
        uint32_t    RecvPing();

        void RunSendPing(std::stop_token token);
        void RunRecvPing(std::stop_token token);

        mutable std::mutex m_mutex;

        unsigned    m_delay;
        bool        m_isRunningRecver;
        bool        m_isRunningSender;
        int         m_sendSocket;
        int         m_recvSocket;
        std::jthread m_sendThread;
        std::jthread m_recvThread;

        PING_MESSAGE m_pmSend;
        uint32_t    m_pid;

        std::string m_errorStr;

        PingIPs m_pingIPs;
};
