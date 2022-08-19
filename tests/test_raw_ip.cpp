#define BOOST_TEST_MODULE STGRawIP

#include "raw_ip_packet_old.h"
#include "stg/raw_ip_packet.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wparentheses"
#include <boost/test/unit_test.hpp>
#pragma GCC diagnostic pop

#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace
{

constexpr size_t ITERATIONS = 1000;

std::array<uint8_t, 68> genVector()
{
    std::array<uint8_t, 68> res;
    for (size_t i = 0; i < res.size(); ++i)
        res[i] = rand() % 256;
    res[0] = (res[0] & 0xF0) | 0x05; // Fix header length
    return res;
}

}

namespace std
{

std::ostream & operator<<(std::ostream& stream, const STG::RawPacket& p)
{
    stream.setf(std::ios::hex);
    for (size_t i = 0; i < sizeof(p.rawPacket.data); ++i) {
        stream << static_cast<unsigned>(p.rawPacket.data[i]);
    }
    stream.setf(std::ios::dec);
    return stream;
}

}

BOOST_AUTO_TEST_SUITE(RawIP)

BOOST_AUTO_TEST_CASE(StructureConsistency)
{
    STG::RawPacket rp;
    rp.rawPacket.header.ipHeader.ip_v = 4;
    rp.rawPacket.header.ipHeader.ip_hl = 5;
    rp.rawPacket.header.ipHeader.ip_tos = 0;
    rp.rawPacket.header.ipHeader.ip_len = htons(40); // 20 of header + 20 of data
    rp.rawPacket.header.ipHeader.ip_p = 6;
    rp.rawPacket.header.ipHeader.ip_src.s_addr = inet_addr("192.168.0.1");
    rp.rawPacket.header.ipHeader.ip_dst.s_addr = inet_addr("192.168.0.101");
    rp.rawPacket.header.sPort = htons(80);
    rp.rawPacket.header.dPort = htons(38546);

    BOOST_CHECK_EQUAL(sizeof(rp.rawPacket.header.ipHeader), static_cast<size_t>(20));
    BOOST_CHECK_EQUAL(rp.GetIPVersion(), 4);
    BOOST_CHECK_EQUAL(rp.GetHeaderLen(), 20);
    BOOST_CHECK_EQUAL(rp.GetProto(), 6);
    BOOST_CHECK_EQUAL(rp.GetLen(), static_cast<uint32_t>(40));
    BOOST_CHECK_EQUAL(rp.GetSrcIP(), inet_addr("192.168.0.1"));
    BOOST_CHECK_EQUAL(rp.GetDstIP(), inet_addr("192.168.0.101"));
    BOOST_CHECK_EQUAL(rp.GetSrcPort(), 80);
    BOOST_CHECK_EQUAL(rp.GetDstPort(), 38546);
}

BOOST_AUTO_TEST_CASE(RandomTests)
{
    srand(time(NULL));
    for (size_t i = 0; i < ITERATIONS; ++i)
    {
        RawPacketOld p1;
        STG::RawPacket p2;
        STG::RawPacket p3;

        const auto buf = genVector();

        memcpy(p1.pckt, buf.data(), 68);
        memcpy(p2.rawPacket.data, buf.data(), 68);
        memcpy(p3.rawPacket.data, buf.data(), 68);

        BOOST_CHECK_EQUAL(p1.GetIPVersion(), p2.GetIPVersion());
        BOOST_CHECK_EQUAL(p1.GetHeaderLen(), p2.GetHeaderLen());
        BOOST_CHECK_EQUAL(p1.GetProto(), p2.GetProto());
        BOOST_CHECK_EQUAL(p1.GetSrcIP(), p2.GetSrcIP());
        BOOST_CHECK_EQUAL(p1.GetDstIP(), p2.GetDstIP());
        BOOST_CHECK_EQUAL(p1.GetSrcPort(), p2.GetSrcPort());
        BOOST_CHECK_EQUAL(p1.GetDstPort(), p2.GetDstPort());

        BOOST_CHECK_EQUAL(p2, p3);
        BOOST_CHECK_EQUAL(p3, p2);
    }
}

BOOST_AUTO_TEST_SUITE_END()
