#ifndef RAW_IP_PACKET_H
#define RAW_IP_PACKET_H

#include <netinet/in.h> // for htons
#include <netinet/ip.h> // for struct ip

#include <cstring>

#include "stg_const.h"
#include "common.h"

#define IPv4 (2)

enum { pcktSize = 68 }; //60(max) ip + 8 udp or tcp (part of tcp or udp header to ports)
//-----------------------------------------------------------------------------
struct RAW_PACKET
{
    RAW_PACKET()
        : dataLen(-1)
    {
    memset(pckt, 0, pcktSize);
    }

    RAW_PACKET(const RAW_PACKET & rp)
        : dataLen(rp.dataLen)
    {
    memcpy(pckt, rp.pckt, pcktSize);
    }

uint16_t    GetIPVersion() const;
uint8_t     GetHeaderLen() const;
uint8_t     GetProto() const;
uint32_t    GetLen() const;
uint32_t    GetSrcIP() const;
uint32_t    GetDstIP() const;
uint16_t    GetSrcPort() const;
uint16_t    GetDstPort() const;

bool        operator==(const RAW_PACKET & rvalue) const;
bool        operator!=(const RAW_PACKET & rvalue) const { return !(*this == rvalue); };
bool        operator<(const RAW_PACKET & rvalue) const;

union
    {
    uint8_t     pckt[pcktSize];         // Начало пакета захваченного из сети
    struct
        {
        struct ip   ipHeader;
        // Only for packets without options field
        uint16_t    sPort;
        uint16_t    dPort;
        } __attribute__ ((packed));
    };
int32_t     dataLen;                // Длина IP пакета. Если -1, то использовать длину из заголовка самого пакета.
};
//-----------------------------------------------------------------------------
inline uint16_t RAW_PACKET::GetIPVersion() const
{
return ipHeader.ip_v;
}
//-----------------------------------------------------------------------------
inline uint8_t RAW_PACKET::GetHeaderLen() const
{
return ipHeader.ip_hl * 4;
}
//-----------------------------------------------------------------------------
inline uint8_t RAW_PACKET::GetProto() const
{
return ipHeader.ip_p;
}
//-----------------------------------------------------------------------------
inline uint32_t RAW_PACKET::GetLen() const
{
if (dataLen != -1)
    return dataLen;
return ntohs(ipHeader.ip_len);
}
//-----------------------------------------------------------------------------
inline uint32_t RAW_PACKET::GetSrcIP() const
{
return ipHeader.ip_src.s_addr;
}
//-----------------------------------------------------------------------------
inline uint32_t RAW_PACKET::GetDstIP() const
{
return ipHeader.ip_dst.s_addr;
}
//-----------------------------------------------------------------------------
inline uint16_t RAW_PACKET::GetSrcPort() const
{
if (ipHeader.ip_p == 1) // for icmp proto return port 0
    return 0;
return ntohs(*((uint16_t*)(pckt + ipHeader.ip_hl * 4)));
}
//-----------------------------------------------------------------------------
inline uint16_t RAW_PACKET::GetDstPort() const
{
if (ipHeader.ip_p == 1) // for icmp proto return port 0
    return 0;
return ntohs(*((uint16_t*)(pckt + ipHeader.ip_hl * 4 + 2)));
}
//-----------------------------------------------------------------------------
inline bool RAW_PACKET::operator==(const RAW_PACKET & rvalue) const
{
if (ipHeader.ip_src.s_addr != rvalue.ipHeader.ip_src.s_addr)
    return false;

if (ipHeader.ip_dst.s_addr != rvalue.ipHeader.ip_dst.s_addr)
    return false;

if (ipHeader.ip_p != 1 && rvalue.ipHeader.ip_p != 1)
    {
    if (*((uint16_t *)(pckt + ipHeader.ip_hl * 4)) !=
        *((uint16_t *)(rvalue.pckt + rvalue.ipHeader.ip_hl * 4)))
        return false;

    if (*((uint16_t *)(pckt + ipHeader.ip_hl * 4 + 2)) !=
        *((uint16_t *)(rvalue.pckt + rvalue.ipHeader.ip_hl * 4 + 2)))
        return false;
    }

if (ipHeader.ip_p != rvalue.ipHeader.ip_p)
    return false;

return true;
}
/*//-----------------------------------------------------------------------------
inline bool operator==(const RAW_PACKET & lhs, const RAW_PACKET & rhs) 
{
if (lhs.GetSrcIP() != rhs.GetSrcIP())
    return false;

if (lhs.GetDstIP() != rhs.GetDstIP())
    return false;

if (lhs.GetSrcPort() != rhs.GetSrcPort())
    return false;

if (lhs.GetDstPort() != rhs.GetDstPort())
    return false;

if (lhs.GetProto() != rhs.GetProto())
    return false;

return true;
}*/
//-----------------------------------------------------------------------------
inline bool RAW_PACKET::operator<(const RAW_PACKET & rvalue) const
{
if (ipHeader.ip_src.s_addr < rvalue.ipHeader.ip_src.s_addr) 
    return true;
if (ipHeader.ip_src.s_addr > rvalue.ipHeader.ip_src.s_addr) 
    return false;

if (ipHeader.ip_dst.s_addr < rvalue.ipHeader.ip_dst.s_addr) 
    return true;
if (ipHeader.ip_dst.s_addr > rvalue.ipHeader.ip_dst.s_addr) 
    return false;

if (ipHeader.ip_p != 1 && rvalue.ipHeader.ip_p != 1)
    {
    if (*((uint16_t *)(pckt + ipHeader.ip_hl * 4)) <
        *((uint16_t *)(rvalue.pckt + rvalue.ipHeader.ip_hl * 4))) 
        return true;
    if (*((uint16_t *)(pckt + ipHeader.ip_hl * 4)) >
        *((uint16_t *)(rvalue.pckt + rvalue.ipHeader.ip_hl * 4))) 
        return false;

    if (*((uint16_t *)(pckt + ipHeader.ip_hl * 4 + 2)) <
        *((uint16_t *)(rvalue.pckt + rvalue.ipHeader.ip_hl * 4 + 2))) 
        return true;
    if (*((uint16_t *)(pckt + ipHeader.ip_hl * 4 + 2)) >
        *((uint16_t *)(rvalue.pckt + rvalue.ipHeader.ip_hl * 4 + 2))) 
        return false;
    }

if (ipHeader.ip_p < rvalue.ipHeader.ip_p) 
    return true;

return false;
}
//-----------------------------------------------------------------------------
/*inline bool operator<(const RAW_PACKET & lhs, const RAW_PACKET & rhs)
{
if (lhs.GetSrcIP() < rhs.GetSrcIP()) 
    return true;
if (lhs.GetSrcIP() > rhs.GetSrcIP()) 
    return false;

if (lhs.GetDstIP() < rhs.GetDstIP()) 
    return true;
if (lhs.GetDstIP() > rhs.GetDstIP()) 
    return false;

if (lhs.GetSrcPort() < rhs.GetSrcPort()) 
    return true;
if (lhs.GetSrcPort() > rhs.GetSrcPort()) 
    return false;

if (lhs.GetDstPort() < rhs.GetDstPort()) 
    return true;
if (lhs.GetDstPort() > rhs.GetDstPort()) 
    return false;

if (lhs.GetProto() < rhs.GetProto()) 
    return true;

return false;
}*/
//-----------------------------------------------------------------------------

#endif


