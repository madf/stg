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

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <arpa/inet.h>

#include "rules.h"
#include "rules_finder.h"
#include "logger.h"

using namespace std;
using namespace STG;

STGLogger log;

RULE MakeRule(const std::string & ip,
              const std::string & mask,
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

PENDING_PACKET MakePacket(const std::string & from,
                          const std::string & to,
                          uint16_t sport,
                          uint16_t dport,
                          int proto,
                          PENDING_PACKET::DIRECTION direction,
                          int length)
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

    PENDING_PACKET packet(hdr, sport, dport);

    packet.direction = direction;

    return packet;
}

struct TEST_INFO {
    int  wantedDir;
    int  actualDir; // Parser error status
    bool stdException; // Parser throws an std execption
    bool otherException; // Parser throws another exception
    bool result;
};

struct RF_TESTER : public std::unary_function<std::pair<PENDING_PACKET, int>, void>
{
public:
    RF_TESTER(RULES_FINDER & r)
        : rf(r),
          testLog(),
          result(true)
        {
        };
    ~RF_TESTER()
        {
        PrintLog();
        if (result)
            exit(EXIT_SUCCESS);
        exit(EXIT_FAILURE);
        }
    void operator()(const std::pair<PENDING_PACKET, int> & entry)
        {
        TEST_INFO info;
        info.wantedDir = entry.second;
        info.actualDir = -1;
        info.stdException = false;
        info.otherException = false;
        info.result = true;
        try
            {
            info.actualDir = rf.GetDir(entry.first);
            }
        catch (std::exception & ex)
            {
            info.stdException = true;
            info.result = false;
            }
        catch (...)
            {
            info.otherException = true;
            info.result = false;
            }
        info.result &= (info.actualDir == info.wantedDir);
        result &= info.result;
        testLog.push_back(info);
        };

    void PrintLog()
        {
        int testNumber = 1;
        std::cout << "RF_TESTER results:\n";
        std::cout << "-----------------------------------------------------------------\n";
        std::vector<TEST_INFO>::const_iterator it;
        for (it = testLog.begin(); it != testLog.end(); ++it)
            {
            std::cout << "Test no.: " << testNumber++ << "\t"
                      << "Correct dir: " << it->wantedDir << "\t"
                      << "Actual dir:" << it->actualDir << "\t"
                      << "STD exceptions: " << it->stdException << "\t"
                      << "Other exceptions: " << it->otherException << "\t"
                      << "Result: " << it->result << "\n";
            }
        std::cout << "-----------------------------------------------------------------\n";
        std::cout << "Final result: " << (result ? "passed" : "failed") << std::endl;
        }

    bool Result() const { return result; };
private:
    RULES_FINDER & rf;
    std::vector<TEST_INFO> testLog;
    bool result;
};

int main()
{
    RULES rules(PrepareRules());
    RULES_FINDER rf;

    rf.SetRules(rules);

    std::list<std::pair<PENDING_PACKET, int> > tests;

    // Local, SSH
    tests.push_back(make_pair(MakePacket("192.168.0.2", "192.168.0.1", 3214, 22, 6, PENDING_PACKET::OUTGOING, 0), 0));
    tests.push_back(make_pair(MakePacket("192.168.0.1", "192.168.0.2", 22, 3214, 6, PENDING_PACKET::OUTGOING, 0), 0));
    // Local, SSH, incorrect direction detection
    tests.push_back(make_pair(MakePacket("192.168.0.2", "192.168.0.1", 3214, 22, 6, PENDING_PACKET::INCOMING, 0), 0));
    tests.push_back(make_pair(MakePacket("192.168.0.1", "192.168.0.2", 22, 3214, 6, PENDING_PACKET::INCOMING, 0), 0));
    // Local, FTP
    tests.push_back(make_pair(MakePacket("192.168.0.2", "192.168.0.1", 3214, 20, 6, PENDING_PACKET::OUTGOING, 0), 0));
    tests.push_back(make_pair(MakePacket("192.168.0.1", "192.168.0.2", 21, 3214, 6, PENDING_PACKET::OUTGOING, 0), 0));
    // Local, DNS
    tests.push_back(make_pair(MakePacket("192.168.0.2", "192.168.0.1", 3214, 53, 6, PENDING_PACKET::OUTGOING, 0), 0));
    tests.push_back(make_pair(MakePacket("192.168.0.1", "192.168.0.2", 53, 3214, 6, PENDING_PACKET::OUTGOING, 0), 0));
    // Known DNS, DNS
    tests.push_back(make_pair(MakePacket("192.168.0.2", "195.5.61.68", 3210, 53, 6, PENDING_PACKET::OUTGOING, 0), 1));
    tests.push_back(make_pair(MakePacket("195.5.61.68", "192.168.0.2", 53, 3210, 6, PENDING_PACKET::INCOMING, 0), 1));
    // Known DNS, invalid ports
    tests.push_back(make_pair(MakePacket("192.168.0.2", "195.5.61.68", 3210, 54, 6, PENDING_PACKET::OUTGOING, 0), 3));
    tests.push_back(make_pair(MakePacket("195.5.61.68", "192.168.0.2", 20, 3210, 6, PENDING_PACKET::INCOMING, 0), 3));
    // Known FTP, FTP
    tests.push_back(make_pair(MakePacket("192.168.0.2", "129.22.8.159", 3241, 20, 6, PENDING_PACKET::OUTGOING, 0), 2));
    tests.push_back(make_pair(MakePacket("129.22.8.159", "192.168.0.2", 21, 3241, 6, PENDING_PACKET::INCOMING, 0), 2));
    // Known FTP, invalid ports
    tests.push_back(make_pair(MakePacket("192.168.0.2", "129.22.8.159", 3241, 53, 6, PENDING_PACKET::OUTGOING, 0), 3));
    tests.push_back(make_pair(MakePacket("129.22.8.159", "192.168.0.2", 22, 3241, 6, PENDING_PACKET::INCOMING, 0), 3));

    std::for_each(tests.begin(),
                 tests.end(),
                 RF_TESTER(rf));

    return EXIT_SUCCESS;
}
