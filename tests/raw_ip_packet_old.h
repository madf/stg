#pragma once

#include "stg/const.h"

#include <cstring>
#include <cstdint>

#include <netinet/in.h> // for htons

enum { pcktSizeOLD = 68 }; //60(max) ip + 8 udp or tcp (part of tcp or udp header to ports)
//-----------------------------------------------------------------------------
struct RawPacketOld
{
    RawPacketOld()
        : dataLen(-1)
    {
        memset(pckt, 0, pcktSizeOLD);
    }

    RawPacketOld(const RawPacketOld& rp)
        : dataLen(rp.dataLen)
    {
        memcpy(pckt, rp.pckt, pcktSizeOLD);
    }

    uint16_t GetIPVersion() const
    {
        return pckt[0] >> 4;
    }
    uint8_t  GetHeaderLen() const
    {
        return (pckt[0] & 0x0F) * 4;
    }
    uint8_t  GetProto() const
    {
        return pckt[9];
    }
    uint32_t GetLen() const
    {
        if (dataLen != -1)
            return dataLen;
        return ntohs(*reinterpret_cast<const uint16_t*>(pckt + 2));
    }
    uint32_t GetSrcIP() const
    {
        return *reinterpret_cast<const uint32_t*>(pckt + 12);
    }
    uint32_t GetDstIP() const
    {
        return *reinterpret_cast<const uint32_t*>(pckt + 16);
    }
    uint16_t GetSrcPort() const
    {
        if (GetProto() == 1) // for icmp proto return port 0
            return 0;
        return ntohs(*reinterpret_cast<const uint16_t*>(pckt + GetHeaderLen()));
    }
    uint16_t GetDstPort() const
    {
        if (GetProto() == 1) // for icmp proto return port 0
            return 0;
        return ntohs(*reinterpret_cast<const uint16_t*>(pckt + GetHeaderLen() + 2));
    }

    uint8_t  pckt[pcktSizeOLD];         // Начало пакета захваченного из сети
    int32_t  dataLen;                // Длина IP пакета. Если -1, то использовать длину из заголовка самого пакета.
};
//-----------------------------------------------------------------------------
inline bool operator==(const RawPacketOld& lhs, const RawPacketOld& rhs)
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
inline bool operator<(const RawPacketOld& lhs, const RawPacketOld& rhs)
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
