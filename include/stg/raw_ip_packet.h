#pragma once

#include <cstring>

#if defined(FREE_BSD)
#include <sys/types.h> // u_char, u_int32_t, etc.
#include <netinet/in_systm.h> // n_long in netinet/ip.h
#endif

#include <netinet/in.h> // for htons
#include <netinet/ip.h> // for struct ip

#define IPv4 (2)

namespace STG
{

enum { packetSize = 68 }; //60(max) ip + 8 udp or tcp (part of tcp or udp header to ports)
//-----------------------------------------------------------------------------
struct RawPacket
{
    RawPacket()
        : dataLen(-1)
    {
        memset(rawPacket.data, 0, packetSize);
    }

    RawPacket(const RawPacket& rhs) noexcept
    {
        memcpy(rawPacket.data, rhs.rawPacket.data, packetSize);
    }
    RawPacket& operator=(const RawPacket& rhs) noexcept
    {
        memcpy(rawPacket.data, rhs.rawPacket.data, packetSize);
        return *this;
    }
    RawPacket(RawPacket&& rhs) noexcept
    {
        memcpy(rawPacket.data, rhs.rawPacket.data, packetSize);
    }
    RawPacket& operator=(RawPacket&& rhs) noexcept
    {
        memcpy(rawPacket.data, rhs.rawPacket.data, packetSize);
        return *this;
    }

    uint16_t GetIPVersion() const noexcept;
    uint8_t  GetHeaderLen() const noexcept;
    uint8_t  GetProto() const noexcept;
    uint32_t GetLen() const noexcept;
    uint32_t GetSrcIP() const noexcept;
    uint32_t GetDstIP() const noexcept;
    uint16_t GetSrcPort() const noexcept;
    uint16_t GetDstPort() const noexcept;

    bool     operator==(const RawPacket& rhs) const noexcept;
    bool     operator!=(const RawPacket& rhs) const noexcept { return !(*this == rhs); }
    bool     operator<(const RawPacket& rhs) const noexcept;

    union
    {
        uint8_t data[packetSize]; // Packet header as a raw data
        struct
        {
            struct ip   ipHeader;
            // Only for packets without options field
            uint16_t    sPort;
            uint16_t    dPort;
        } header;
    } rawPacket;
    int32_t dataLen; // IP packet length. Set to -1 to use length field from the header
};
//-----------------------------------------------------------------------------
inline uint16_t RawPacket::GetIPVersion() const noexcept
{
    return rawPacket.header.ipHeader.ip_v;
}
//-----------------------------------------------------------------------------
inline uint8_t RawPacket::GetHeaderLen() const noexcept
{
    return rawPacket.header.ipHeader.ip_hl * 4;
}
//-----------------------------------------------------------------------------
inline uint8_t RawPacket::GetProto() const noexcept
{
    return rawPacket.header.ipHeader.ip_p;
}
//-----------------------------------------------------------------------------
inline uint32_t RawPacket::GetLen() const noexcept
{
    if (dataLen != -1)
        return dataLen;
    return ntohs(rawPacket.header.ipHeader.ip_len);
}
//-----------------------------------------------------------------------------
inline uint32_t RawPacket::GetSrcIP() const noexcept
{
    return rawPacket.header.ipHeader.ip_src.s_addr;
}
//-----------------------------------------------------------------------------
inline uint32_t RawPacket::GetDstIP() const noexcept
{
    return rawPacket.header.ipHeader.ip_dst.s_addr;
}
//-----------------------------------------------------------------------------
inline uint16_t RawPacket::GetSrcPort() const noexcept
{
    if (rawPacket.header.ipHeader.ip_p == 1) // for icmp proto return port 0
        return 0;
    const uint8_t* pos = rawPacket.data + rawPacket.header.ipHeader.ip_hl * 4;
    return ntohs(*reinterpret_cast<const uint16_t *>(pos));
}
//-----------------------------------------------------------------------------
inline uint16_t RawPacket::GetDstPort() const noexcept
{
    if (rawPacket.header.ipHeader.ip_p == 1) // for icmp proto return port 0
        return 0;
    const uint8_t * pos = rawPacket.data + rawPacket.header.ipHeader.ip_hl * 4 + 2;
    return ntohs(*reinterpret_cast<const uint16_t *>(pos));
}
//-----------------------------------------------------------------------------
inline bool RawPacket::operator==(const RawPacket& rhs) const noexcept
{
    if (rawPacket.header.ipHeader.ip_src.s_addr != rhs.rawPacket.header.ipHeader.ip_src.s_addr)
        return false;

    if (rawPacket.header.ipHeader.ip_dst.s_addr != rhs.rawPacket.header.ipHeader.ip_dst.s_addr)
        return false;

    if (rawPacket.header.ipHeader.ip_p != 1 && rhs.rawPacket.header.ipHeader.ip_p != 1)
    {
        const uint8_t * pos = rawPacket.data + rawPacket.header.ipHeader.ip_hl * 4;
        const uint8_t * rpos = rhs.rawPacket.data + rhs.rawPacket.header.ipHeader.ip_hl * 4;
        if (*reinterpret_cast<const uint16_t *>(pos) != *reinterpret_cast<const uint16_t *>(rpos))
            return false;

        pos += 2;
        rpos += 2;
        if (*reinterpret_cast<const uint16_t *>(pos) != *reinterpret_cast<const uint16_t *>(rpos))
            return false;
    }

    if (rawPacket.header.ipHeader.ip_p != rhs.rawPacket.header.ipHeader.ip_p)
        return false;

    return true;
}
//-----------------------------------------------------------------------------
inline bool RawPacket::operator<(const RawPacket& rhs) const noexcept
{
    if (rawPacket.header.ipHeader.ip_src.s_addr < rhs.rawPacket.header.ipHeader.ip_src.s_addr)
        return true;
    if (rawPacket.header.ipHeader.ip_src.s_addr > rhs.rawPacket.header.ipHeader.ip_src.s_addr)
        return false;

    if (rawPacket.header.ipHeader.ip_dst.s_addr < rhs.rawPacket.header.ipHeader.ip_dst.s_addr)
        return true;
    if (rawPacket.header.ipHeader.ip_dst.s_addr > rhs.rawPacket.header.ipHeader.ip_dst.s_addr)
        return false;

    if (rawPacket.header.ipHeader.ip_p != 1 && rhs.rawPacket.header.ipHeader.ip_p != 1)
    {
        const uint8_t * pos = rawPacket.data + rawPacket.header.ipHeader.ip_hl * 4;
        const uint8_t * rpos = rhs.rawPacket.data + rhs.rawPacket.header.ipHeader.ip_hl * 4;
        if (*reinterpret_cast<const uint16_t *>(pos) < *reinterpret_cast<const uint16_t *>(rpos))
            return true;
        if (*reinterpret_cast<const uint16_t *>(pos) > *reinterpret_cast<const uint16_t *>(rpos))
            return false;

        pos += 2;
        rpos += 2;
        if (*reinterpret_cast<const uint16_t *>(pos) < *reinterpret_cast<const uint16_t *>(rpos))
            return true;
        if (*reinterpret_cast<const uint16_t *>(pos) > *reinterpret_cast<const uint16_t *>(rpos))
            return false;
    }

    if (rawPacket.header.ipHeader.ip_p < rhs.rawPacket.header.ipHeader.ip_p)
        return true;

    return false;
}
//-----------------------------------------------------------------------------

}
