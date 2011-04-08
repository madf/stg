#ifndef RAW_IP_PACKET_OLD_H
#define RAW_IP_PACKET_OLD_H

#include <netinet/in.h> // for htons
//#include <netinet/ip.h> // for struct ip

#include <cstring>

#include "const.h"

#define IPv4 (2)

enum { pcktSizeOLD = 68 }; //60(max) ip + 8 udp or tcp (part of tcp or udp header to ports)
//-----------------------------------------------------------------------------
struct RAW_PACKET_OLD
{
    RAW_PACKET_OLD()
        : dataLen(-1)
    {
    memset(pckt, 0, pcktSizeOLD);
    }

    RAW_PACKET_OLD(const RAW_PACKET_OLD & rp)
        : dataLen(rp.dataLen)
    {
    memcpy(pckt, rp.pckt, pcktSizeOLD);
    }

uint16_t    GetIPVersion() const;
uint8_t     GetHeaderLen() const;
uint8_t     GetProto() const;
uint32_t    GetLen() const;
uint32_t    GetSrcIP() const;
uint32_t    GetDstIP() const;
uint16_t    GetSrcPort() const;
uint16_t    GetDstPort() const;

uint8_t     pckt[pcktSizeOLD];         // Начало пакета захваченного из сети
int32_t     dataLen;                // Длина IP пакета. Если -1, то использовать длину из заголовка самого пакета.
};
//-----------------------------------------------------------------------------
inline uint16_t RAW_PACKET_OLD::GetIPVersion() const
{
return pckt[0] >> 4;
}
//-----------------------------------------------------------------------------
inline uint8_t RAW_PACKET_OLD::GetHeaderLen() const
{
return (pckt[0] & 0x0F) * 4;
}
//-----------------------------------------------------------------------------
inline uint8_t RAW_PACKET_OLD::GetProto() const
{
return pckt[9];
}
//-----------------------------------------------------------------------------
inline uint32_t RAW_PACKET_OLD::GetLen() const
{
if (dataLen != -1)
    return dataLen;
return ntohs(*(uint16_t*)(pckt + 2));
}
//-----------------------------------------------------------------------------
inline uint32_t RAW_PACKET_OLD::GetSrcIP() const
{
return *(uint32_t*)(pckt + 12);
}
//-----------------------------------------------------------------------------
inline uint32_t RAW_PACKET_OLD::GetDstIP() const
{
return *(uint32_t*)(pckt + 16);
}
//-----------------------------------------------------------------------------
inline uint16_t RAW_PACKET_OLD::GetSrcPort() const
{
if (GetProto() == 1) // for icmp proto return port 0
    return 0;
return ntohs(*((uint16_t*)(pckt + GetHeaderLen())));
}
//-----------------------------------------------------------------------------
inline uint16_t RAW_PACKET_OLD::GetDstPort() const
{
if (GetProto() == 1) // for icmp proto return port 0
    return 0;
return ntohs(*((uint16_t*)(pckt + GetHeaderLen() + 2)));
}
//-----------------------------------------------------------------------------
inline bool operator==(const RAW_PACKET_OLD & lhs, const RAW_PACKET_OLD & rhs) 
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
}
//-----------------------------------------------------------------------------
inline bool operator<(const RAW_PACKET_OLD & lhs, const RAW_PACKET_OLD & rhs)
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

/*
Last compare

if (lhs.GetProto() > rhs.GetProto())
    return false;

don't needed
*/

return false;
}
//-----------------------------------------------------------------------------

#endif
