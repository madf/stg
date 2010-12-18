#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cstdlib>
#include <ctime>
#include <iostream>

#include "tut/tut.hpp"

#include "raw_ip_packet_old.h"
#include "raw_ip_packet.h"

#ifndef ITERATIONS
#define ITERATIONS 1000
#endif

void genVector(uint8_t * buf);

std::ostream & operator<<(std::ostream & stream, const RAW_PACKET & p);

namespace tut
{
    struct rp_data {
    };

    typedef test_group<rp_data> tg;
    tg rp_test_group("RAW_PACKET tests group");

    typedef tg::object testobject;

    template<>
    template<>
    void testobject::test<1>()
    {
        set_test_name("Check structure consistency");

        RAW_PACKET rp;
        rp.ipHeader.ip_v = 4;
        rp.ipHeader.ip_hl = 5;
        rp.ipHeader.ip_tos = 0;
        rp.ipHeader.ip_len = htons(40); // 20 of header + 20 of data
        rp.ipHeader.ip_p = 6;
        rp.ipHeader.ip_src.s_addr = inet_addr("192.168.0.1");
        rp.ipHeader.ip_dst.s_addr = inet_addr("192.168.0.101");
        rp.sPort = htons(80);
        rp.dPort = htons(38546);

        ensure_equals("IP header size (explicitly)", sizeof(rp.ipHeader), 20);
        ensure_equals("IP version", rp.GetIPVersion(), 4);
        ensure_equals("IP header size (with options)", rp.GetHeaderLen(), 20);
        ensure_equals("Underlying protocol version", rp.GetProto(), 6);
        ensure_equals("Packet length", rp.GetLen(), 40);
        ensure_equals("Source IP address", rp.GetSrcIP(), inet_addr("192.168.0.1"));
        ensure_equals("Destination IP address", rp.GetDstIP(), inet_addr("192.168.0.101"));
        ensure_equals("Source port number", rp.GetSrcPort(), 80);
        ensure_equals("Destination port number", rp.GetDstPort(), 38546);
    }

    template<>
    template<>
    void testobject::test<2>()
    {
        srand(time(NULL));
        for (size_t i = 0; i < ITERATIONS; ++i) {
            RAW_PACKET_OLD p1;
            RAW_PACKET p2;
            RAW_PACKET p3;

            uint8_t buf[68];
            genVector(buf);

            memcpy(p1.pckt, buf, 68);
            memcpy(p2.pckt, buf, 68);
            memcpy(p3.pckt, buf, 68);

            ensure_equals("IP versions", p1.GetIPVersion(), p2.GetIPVersion());
            ensure_equals("IP headers length", p1.GetHeaderLen(), p2.GetHeaderLen());
            ensure_equals("Protocols", p1.GetProto(), p2.GetProto());
            ensure_equals("Source IPs", p1.GetSrcIP(), p2.GetSrcIP());
            ensure_equals("Destination IPs", p1.GetDstIP(), p2.GetDstIP());
            ensure_equals("Source ports", p1.GetSrcPort(), p2.GetSrcPort());
            ensure_equals("Destination ports", p1.GetDstPort(), p2.GetDstPort());

            ensure_equals("Self equallity", p2, p3);
            ensure_equals("Reverse self equallity", p3, p2);
        }
    }
}

inline
void genVector(uint8_t * buf)
{
    for (size_t i = 0; i < 68; ++i) {
        buf[i] = rand() % 256;
    }
    buf[0] = (buf[0] & 0xF0) | 0x05; // Fix header length
}

std::ostream & operator<<(std::ostream & stream, const RAW_PACKET & p)
{
    stream.unsetf(std::ios::dec);
    stream.setf(std::ios::hex);
    for (size_t i = 0; i < sizeof(p.pckt); ++i) {
        stream << static_cast<unsigned>(p.pckt[i]);
    }
    stream.unsetf(std::ios::hex);
    stream.setf(std::ios::dec);
    return stream;
}
