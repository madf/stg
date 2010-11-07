/*
 *  Network:
 *   - server: 192.168.0.1
 *   - user A: 192.168.0.2
 *   - user B: 192.168.0.3
 *
 *  External resources:
 *   - host 1: 216.239.59.104
 *   - host 2: 72.14.221.104
 *   - host 3: 66.249.93.104
 *   - host 4: 195.5.61.68
 *   
 *  Directions:
 *   - Local: ALL 192.168.0.0/24
 *   - DNS: TCP_UDP 195.5.61.68/32:53
 *   - FTP: TCP 129.22.8.159/32:20-21
 *   - World: ALL 0.0.0.0/0
 *
 */



#include <iostream>
#include <algorithm>
#include <functional>

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/time.h>

#include "rules.h"
#include "traffcounter.h"
#include "logger.h"

using namespace std;
using namespace STG;

class StatPrinter: public unary_function<const TRAFF_ITEM &, void> {
public:
    void operator()(const TRAFF_ITEM & item) const
    {
        LOG_IT << inet_ntoa(*(in_addr *)(&item.first.saddr));
        cout   << ":" << item.first.sport;
        cout   << " -> " << inet_ntoa(*(in_addr *)(&item.first.daddr));
        cout   << ":" << item.first.dport;
        cout   << "\tproto: " << item.first.proto;
        cout   << "\tlength: " << item.second.length;
        cout   << endl;
    }
};

STGLogger log;

struct PACKET
{
    iphdr hdr;
    uint16_t sport;
    uint16_t dport;
};

RULE MakeRule(const string & ip,
              const string & mask,
              uint16_t port1,
              uint16_t port2,
              int proto,
              int dir)
{
    RULE rule;

    rule.ip = inet_addr(ip.c_str());
    rule.mask = inet_addr(mask.c_str());
    rule.port1 = port1;
    rule.port2 = port2;
    rule.proto = proto;
    rule.dir = dir;

    return rule;
}

RULES PrepareRules()
{
    RULES rules;
    RULE local(MakeRule("192.168.0.0",
                        "255.255.255.0",
                        0,
                        65535,
                        -1,
                        0));
    RULE dns(MakeRule("195.5.61.68",
                      "255.255.255.255",
                      53,
                      53,
                      -1,
                      1));
    RULE ftp(MakeRule("129.22.8.159",
                      "255.255.255.255",
                      20,
                      21,
                      -1,
                      2));
    RULE world(MakeRule("0.0.0.0",
                        "0.0.0.0",
                        0,
                        65535,
                        -1,
                        3));

    rules.push_back(local);

    rules.push_back(dns);

    rules.push_back(ftp);

    rules.push_back(world);

    return rules;
}

iphdr MakePacket(const string & from,
                 const string & to,
                 int proto,
                 uint16_t length)
{
    iphdr hdr;

    hdr.ihl = 5;
    hdr.version = 4;
    hdr.tos = 0;
    hdr.tot_len = length;
    hdr.id = 0;
    hdr.frag_off = 50;
    hdr.ttl = 64;
    hdr.protocol = proto;
    hdr.check = 0;
    hdr.saddr = inet_addr(from.c_str());
    hdr.daddr = inet_addr(to.c_str());

    return hdr;
}

void PrepareTraffic(vector<PACKET> & pckts,
                    const iphdr & skel,
                    uint16_t sport,
                    uint16_t dport,
                    uint32_t in,
                    uint32_t out,
                    int packets)
{
    PACKET inpacket;
    PACKET outpacket;

    inpacket.hdr = skel;
    outpacket.hdr = skel;

    outpacket.hdr.saddr ^= outpacket.hdr.daddr;
    outpacket.hdr.daddr ^= outpacket.hdr.saddr;
    outpacket.hdr.saddr ^= outpacket.hdr.daddr;

    inpacket.sport = sport;
    inpacket.dport = dport;
    outpacket.sport = dport;
    outpacket.dport = sport;

    inpacket.hdr.tot_len = in / packets;
    outpacket.hdr.tot_len = out / packets;

    for (int i = 0; i < packets; ++i) {
        //inpacket.hdr.daddr = outpacket.hdr.saddr = rand() * 32768 + rand();
        pckts.push_back(inpacket);
        pckts.push_back(outpacket);
    }
}

struct TC_TESTER : public unary_function<const PACKET &, void>
{
public:
    TC_TESTER(TRAFFCOUNTER & t)
        : tc(t)
    {}

    void operator () (const PACKET & p)
    {
        tc.AddPacket(p.hdr, p.sport, p.dport);
    }
private:
    TRAFFCOUNTER & tc;
};

int main()
{
    RULES rules(PrepareRules());

    TRAFFCOUNTER tc;

    vector<PACKET> packets;

    tc.SetRules(rules);

    if (tc.Start()) {
        LOG_IT << "::main() Error: traffcounter not started" << endl;
        return EXIT_FAILURE;
    }

    tc.AddIP(inet_addr("192.168.0.1")); // Server
    tc.AddIP(inet_addr("192.168.0.2")); // User A
    tc.AddIP(inet_addr("192.168.0.3")); // User B

    for (int i = 0; i < 10000; ++i) {
        tc.AddIP(rand() * 32768 + rand());
    }

    /*
     * A -> S
     * S -> A
     * A -> B
     * B -> A
     * A -> h1
     * h1 -> A
     * A -> h2
     * h2 -> A
     * A -> h3
     * h3 -> A
     * A -> h4
     * h4 -> A
     */

    timeval tv_from;
    timeval tv_to;
    gettimeofday(&tv_from, NULL);
    // S - local, A - local
    PrepareTraffic(packets, MakePacket("192.168.0.2", "192.168.0.1", 6, 0), 3215, 20, 1024 * 1024, 2048 * 1024, 512 * 2);
    // S - local, B - local
    PrepareTraffic(packets, MakePacket("192.168.0.3", "192.168.0.1", 6, 0), 5432, 22, 2048 * 1024, 1024 * 1024, 512 * 2);
    // A - local, B - local
    PrepareTraffic(packets, MakePacket("192.168.0.3", "192.168.0.2", 6, 0), 9875, 21, 2048 * 1024, 2048 * 1024, 512 * 2);
    // A - DNS
    //PrepareTraffic(packets, MakePacket("192.168.0.2", "195.5.61.68", 6, 0), 4521, 53, 1024 * 1024, 2048 * 1024, 512 * 2);
    // A - World
    //PrepareTraffic(packets, MakePacket("192.168.0.2", "195.5.61.68", 6, 0), 4521, 80, 1024 * 1024, 2048 * 1024, 512 * 2);
    // A - FTP
    //PrepareTraffic(packets, MakePacket("192.168.0.2", "129.22.8.159", 6, 0), 4522, 20, 512 * 1024, 512  * 1024, 512 * 2);
    // A - FTP
    //PrepareTraffic(packets, MakePacket("192.168.0.2", "129.22.8.159", 6, 0), 4522, 21, 512 * 1024, 4096 * 1024, 512 * 2);
    // B - World
    //PrepareTraffic(packets, MakePacket("192.168.0.3", "66.249.93.104", 6, 0), 3541, 80, 1024 * 1024, 1024 * 1024, 512 * 2);
    gettimeofday(&tv_to, NULL);

    uint64_t diff = tv_to.tv_sec - tv_from.tv_sec;
    diff *= 1000000;
    diff -= tv_from.tv_usec;
    diff += tv_to.tv_usec;

    LOG_IT << "::main() Prepared " << packets.size() << " packets by " << diff << " usecs" << endl;

    gettimeofday(&tv_from, NULL);
    for_each(packets.begin(),
                  packets.end(),
                  TC_TESTER(tc));
    gettimeofday(&tv_to, NULL);

    diff = tv_to.tv_sec - tv_from.tv_sec;
    diff *= 1000000;
    diff -= tv_from.tv_usec;
    diff += tv_to.tv_usec;

    LOG_IT << "::main() Recorded " << packets.size() << " packets by " << diff << " usecs" << endl;

    int p;
    while ((p = tc.PendingCount())) {
        LOG_IT << "::main() Pending packets: " << p << " at " << time(NULL) << endl;
        sleep(1);
    }

    TRAFF_DATA data;

    tc.DeleteIP(inet_addr("192.168.0.1"), &data);
    for_each(data.begin(),
                  data.end(),
                  StatPrinter());
    data.erase(data.begin(), data.end());
    tc.DeleteIP(inet_addr("192.168.0.2"), &data);
    for_each(data.begin(),
                  data.end(),
                  StatPrinter());
    data.erase(data.begin(), data.end());
    tc.DeleteIP(inet_addr("192.168.0.3"), &data);
    for_each(data.begin(),
                  data.end(),
                  StatPrinter());
    data.erase(data.begin(), data.end());

    if (tc.Stop()) {
        LOG_IT << "::main() Error: traffcounter not stopped" << endl;
        return EXIT_FAILURE;
    }

    LOG_IT << "::main() Sessions: " << tc.SessionsCount() << endl;
    LOG_IT << "::main() Cache hits: " << tc.CacheHits() << endl;
    LOG_IT << "::main() Cache misses: " << tc.CacheMisses() << endl;
    LOG_IT << "::main() Stream quality: " << tc.StreamQuality() << endl;

    return EXIT_SUCCESS;
}
