/*
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *    Date: 27.10.2002
 */

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

/*
 $Revision: 1.58 $
 $Date: 2010/11/03 11:28:07 $
 $Author: faust $
 */

/* inet_aton */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <csignal>
#include <cassert>
#include <cstdio> // fopen and similar

#include "traffcounter_impl.h"
#include "common.h"
#include "stg_locker.h"
#include "stg_timer.h"

#define FLUSH_TIME  (10)
#define REMOVE_TIME  (31)

const char protoName[PROTOMAX][8] =
{"TCP", "UDP", "ICMP", "TCP_UDP", "ALL"};

enum protoNum
{
tcp = 0, udp, icmp, tcp_udp, all
};

//-----------------------------------------------------------------------------
TRAFFCOUNTER_IMPL::TRAFFCOUNTER_IMPL(USERS * u, const TARIFFS *, const std::string & fn)
    : WriteServLog(GetStgLogger()),
      rulesFileName(fn),
      monitoring(false),
      users(u),
      running(false),
      stopped(true),
      addUserNotifier(*this),
      delUserNotifier(*this)
{
for (int i = 0; i < DIR_NUM; i++)
    strprintf(&dirName[i], "DIR%d", i);

dirName[DIR_NUM] = "NULL";

users->AddNotifierUserAdd(&addUserNotifier);
users->AddNotifierUserDel(&delUserNotifier);

pthread_mutex_init(&mutex, NULL);
}
//-----------------------------------------------------------------------------
TRAFFCOUNTER_IMPL::~TRAFFCOUNTER_IMPL()
{
pthread_mutex_destroy(&mutex);
}
//-----------------------------------------------------------------------------
int TRAFFCOUNTER_IMPL::Start()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (!stopped)
    return 0;

if (ReadRules())
    {
    WriteServLog("TRAFFCOUNTER: Cannot read rules.");
    return -1;
    }

printfd(__FILE__, "TRAFFCOUNTER::Start()\n");
int h = users->OpenSearch();
USER_PTR u;
if (!h)
    {
    WriteServLog("TRAFFCOUNTER: Cannot get users.");
    return -1;
    }

while (users->SearchNext(h, &u) == 0)
    {
    SetUserNotifiers(u);
    }
users->CloseSearch(h);

running = true;
if (pthread_create(&thread, NULL, Run, this))
    {
    WriteServLog("TRAFFCOUNTER: Error: Cannot start thread!");
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int TRAFFCOUNTER_IMPL::Stop()
{
if (stopped)
    return 0;

running = false;

int h = users->OpenSearch();
if (!h)
    {
    WriteServLog("TRAFFCOUNTER: Fatal error: Cannot get users.");
    return -1;
    }

USER_PTR u;
while (users->SearchNext(h, &u) == 0)
    {
    UnSetUserNotifiers(u);
    }
users->CloseSearch(h);

//5 seconds to thread stops itself
struct timespec ts = {0, 200000000};
for (int i = 0; i < 25 && !stopped; i++)
    {
    nanosleep(&ts, NULL);
    }

//after 5 seconds waiting thread still running. now kill it
if (!stopped)
    {
    printfd(__FILE__, "kill TRAFFCOUNTER thread.\n");
    if (pthread_kill(thread, SIGINT))
        {
        return -1;
        }
    printfd(__FILE__, "TRAFFCOUNTER killed\n");
    }
printfd(__FILE__, "TRAFFCOUNTER::Stop()\n");

return 0;
}
//-----------------------------------------------------------------------------
void * TRAFFCOUNTER_IMPL::Run(void * data)
{
TRAFFCOUNTER_IMPL * tc = static_cast<TRAFFCOUNTER_IMPL *>(data);
tc->stopped = false;
int c = 0;

time_t touchTime = stgTime - MONITOR_TIME_DELAY_SEC;
struct timespec ts = {0, 500000000};
while (tc->running)
    {
    nanosleep(&ts, 0);
    if (!tc->running)
        {
        tc->FlushAndRemove();
        break;
        }

    if (tc->monitoring && (touchTime + MONITOR_TIME_DELAY_SEC <= stgTime))
        {
        std::string monFile(tc->monitorDir + "/traffcounter_r");
        printfd(__FILE__, "Monitor=%d file TRAFFCOUNTER %s\n", tc->monitoring, monFile.c_str());
        touchTime = stgTime;
        TouchFile(monFile.c_str());
        }

    if (++c % FLUSH_TIME == 0)
        tc->FlushAndRemove();
    }

tc->stopped = true;
return NULL;
}
//-----------------------------------------------------------------------------
void TRAFFCOUNTER_IMPL::Process(const RAW_PACKET & rawPacket)
{
if (!running)
    return;

static time_t touchTime = stgTime - MONITOR_TIME_DELAY_SEC;

if (monitoring && (touchTime + MONITOR_TIME_DELAY_SEC <= stgTime))
    {
    static std::string monFile = monitorDir + "/traffcounter_p";
    printfd(__FILE__, "Monitor=%d file TRAFFCOUNTER %s\n", monitoring, monFile.c_str());
    touchTime = stgTime;
    TouchFile(monFile.c_str());
    }

STG_LOCKER lock(&mutex, __FILE__, __LINE__);

//printfd(__FILE__, "TRAFFCOUNTER::Process()\n");
//TODO replace find with lower_bound.

// Searching a new packet in a tree.
pp_iter pi = packets.find(rawPacket);

// Packet found - update length and time
if (pi != packets.end())
    {
    pi->second.lenU += rawPacket.GetLen();
    pi->second.lenD += rawPacket.GetLen();
    pi->second.updateTime = stgTime;
    /*printfd(__FILE__, "=============================\n");
    printfd(__FILE__, "Packet found!\n");
    printfd(__FILE__, "Version=%d\n", rawPacket.GetIPVersion());
    printfd(__FILE__, "HeaderLen=%d\n", rawPacket.GetHeaderLen());
    printfd(__FILE__, "PacketLen=%d\n", rawPacket.GetLen());
    printfd(__FILE__, "SIP=%s\n", inet_ntostring(rawPacket.GetSrcIP()).c_str());
    printfd(__FILE__, "DIP=%s\n", inet_ntostring(rawPacket.GetDstIP()).c_str());
    printfd(__FILE__, "src port=%d\n", rawPacket.GetSrcPort());
    printfd(__FILE__, "pst port=%d\n", rawPacket.GetDstPort());
    printfd(__FILE__, "len=%d\n", rawPacket.GetLen());
    printfd(__FILE__, "proto=%d\n", rawPacket.GetProto());
    printfd(__FILE__, "PacketDirU=%d\n", pi->second.dirU);
    printfd(__FILE__, "PacketDirD=%d\n", pi->second.dirD);
    printfd(__FILE__, "=============================\n");*/
    return;
    }

PACKET_EXTRA_DATA ed;

// Packet not found - add new packet

ed.updateTime = stgTime;
ed.flushTime = stgTime;

/*
 userU is that whose user_ip == packet_ip_src
 userD is that whose user_ip == packet_ip_dst
 */

uint32_t ipU = rawPacket.GetSrcIP();
uint32_t ipD = rawPacket.GetDstIP();

// Searching users with such IP
if (users->FindByIPIdx(ipU, &ed.userU) == 0)
    {
    ed.userUPresent = true;
    }

if (users->FindByIPIdx(ipD, &ed.userD) == 0)
    {
    ed.userDPresent = true;
    }

if (ed.userUPresent ||
    ed.userDPresent)
    {
    DeterminateDir(rawPacket, &ed.dirU, &ed.dirD);

    ed.lenD = ed.lenU = rawPacket.GetLen();

    //TODO use result of lower_bound to inserting new record

    // Adding packet to a tree.
    std::pair<pp_iter, bool> insertResult = packets.insert(std::make_pair(rawPacket, ed));
    pp_iter newPacket = insertResult.first;

    // Adding packet reference to an IP index.
    ip2packets.insert(std::make_pair(ipU, newPacket));
    ip2packets.insert(std::make_pair(ipD, newPacket));
    }
}
//-----------------------------------------------------------------------------
void TRAFFCOUNTER_IMPL::FlushAndRemove()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

int oldPacketsSize = packets.size();
int oldIp2packetsSize = ip2packets.size();

pp_iter pi;
pi = packets.begin();
std::map<RAW_PACKET, PACKET_EXTRA_DATA> newPackets;
ip2packets.erase(ip2packets.begin(), ip2packets.end());
while (pi != packets.end())
    {
    //Flushing
    if (stgTime - pi->second.flushTime > FLUSH_TIME)
        {
        if (pi->second.userUPresent)
            {
            //printfd(__FILE__, "+++ Flushing U user %s (%s:%d) of length %d\n", pi->second.userU->GetLogin().c_str(), inet_ntostring(pi->first.GetSrcIP()).c_str(), pi->first.GetSrcPort(), pi->second.lenU);

            // Add stat
            if (pi->second.dirU < DIR_NUM)
                {
                #ifdef TRAFF_STAT_WITH_PORTS
                pi->second.userU->AddTraffStatU(pi->second.dirU,
                                                pi->first.GetDstIP(),
                                                pi->first.GetDstPort(),
                                                pi->second.lenU);
                #else
                pi->second.userU->AddTraffStatU(pi->second.dirU,
                                                pi->first.GetDstIP(),
                                                pi->second.lenU);
                #endif
                }

            pi->second.lenU = 0;
            pi->second.flushTime = stgTime;
            }

        if (pi->second.userDPresent)
            {
            //printfd(__FILE__, "+++ Flushing D user %s (%s:%d) of length %d\n", pi->second.userD->GetLogin().c_str(), inet_ntostring(pi->first.GetDstIP()).c_str(), pi->first.GetDstPort(), pi->second.lenD);

            // Add stat
            if (pi->second.dirD < DIR_NUM)
                {
                #ifdef TRAFF_STAT_WITH_PORTS
                pi->second.userD->AddTraffStatD(pi->second.dirD,
                                                pi->first.GetSrcIP(),
                                                pi->first.GetSrcPort(),
                                                pi->second.lenD);
                #else
                pi->second.userD->AddTraffStatD(pi->second.dirD,
                                                pi->first.GetSrcIP(),
                                                pi->second.lenD);
                #endif
                }

            pi->second.lenD = 0;
            pi->second.flushTime = stgTime;
            }
        }

    /*//Removing
    if (stgTime - pi->second.updateTime > REMOVE_TIME)
        {
        // Remove packet and references from ip2packets index
        //printfd(__FILE__, "+++ Removing +++\n");
        pair<ip2p_iter, ip2p_iter> be(
                ip2packets.equal_range(pi->first.GetSrcIP()));
        while (be.first != be.second)
            {
            // Have a reference to a packet?
            if (be.first->second == pi)
                {
                ip2packets.erase(be.first++);
                //printfd(__FILE__, "Remove U from ip2packets %s\n", inet_ntostring(pi->first.GetSrcIP()).c_str());
                }
            else
                {
                ++be.first;
                }
            }

        //printfd(__FILE__, "-------------------\n");
        be = ip2packets.equal_range(pi->first.GetDstIP());
        while (be.first != be.second)
            {
            // Have a reference to a packet?
            if (be.first->second == pi)
                {
                ip2packets.erase(be.first++);
                //printfd(__FILE__, "Remove D from ip2packets %s\n", inet_ntostring(pi->first.GetDstIP()).c_str());
                }
            else
                {
                ++be.first;
                }
            }
        //printfd(__FILE__, "Remove packet\n");
        packets.erase(pi++);
        }
    else
        {
        ++pi;
        }*/
    if (stgTime - pi->second.updateTime < REMOVE_TIME)
        {
        std::pair<pp_iter, bool> res = newPackets.insert(*pi);
        if (res.second)
            {
            ip2packets.insert(std::make_pair(pi->first.GetSrcIP(), res.first));
            ip2packets.insert(std::make_pair(pi->first.GetDstIP(), res.first));
            }
        }
    ++pi;
    }
swap(packets, newPackets);
printfd(__FILE__, "FlushAndRemove() packets: %d(rem %d) ip2packets: %d(rem %d)\n",
        packets.size(),
        oldPacketsSize - packets.size(),
        ip2packets.size(),
        oldIp2packetsSize - ip2packets.size());

}
//-----------------------------------------------------------------------------
void TRAFFCOUNTER_IMPL::AddUser(USER_PTR user)
{
printfd(__FILE__, "AddUser: %s\n", user->GetLogin().c_str());
uint32_t uip = user->GetCurrIP();
std::pair<ip2p_iter, ip2p_iter> pi;

STG_LOCKER lock(&mutex, __FILE__, __LINE__);
// Find all packets with IP belongs to this user
pi = ip2packets.equal_range(uip);

while (pi.first != pi.second)
    {
    if (pi.first->second->first.GetSrcIP() == uip)
        {
        assert((!pi.first->second->second.userUPresent || 
                 pi.first->second->second.userU == user) &&
               "U user present but it's not current user");

        pi.first->second->second.lenU = 0;
        pi.first->second->second.userU = user;
        pi.first->second->second.userUPresent = true;
        }

    if (pi.first->second->first.GetDstIP() == uip)
        {
        assert((!pi.first->second->second.userDPresent || 
                 pi.first->second->second.userD == user) &&
               "D user present but it's not current user");

        pi.first->second->second.lenD = 0;
        pi.first->second->second.userD = user;
        pi.first->second->second.userDPresent = true;
        }

    ++pi.first;
    }
}
//-----------------------------------------------------------------------------
void TRAFFCOUNTER_IMPL::DelUser(uint32_t uip)
{
printfd(__FILE__, "DelUser: %s \n", inet_ntostring(uip).c_str());
std::pair<ip2p_iter, ip2p_iter> pi;

STG_LOCKER lock(&mutex, __FILE__, __LINE__);
pi = ip2packets.equal_range(uip);

while (pi.first != pi.second)
    {
    if (pi.first->second->first.GetSrcIP() == uip)
        {
        if (pi.first->second->second.dirU < DIR_NUM && pi.first->second->second.userUPresent)
            {
            #ifdef TRAFF_STAT_WITH_PORTS
            pi.first->second->second.userU->AddTraffStatU(pi.first->second->second.dirU,
                                                          pi.first->second->first.GetDstIP(),
                                                          pi.first->second->first.GetDstPort(),
                                                          pi.first->second->second.lenU);
            #else
            pi.first->second->second.userU->AddTraffStatU(pi.first->second->second.dirU,
                                                          pi.first->second->first.GetDstIP(),
                                                          pi.first->second->second.lenU);
            #endif
            }
        pi.first->second->second.userUPresent = false;
        }

    if (pi.first->second->first.GetDstIP() == uip)
        {
        if (pi.first->second->second.dirD < DIR_NUM && pi.first->second->second.userDPresent)
            {
            #ifdef TRAFF_STAT_WITH_PORTS
            pi.first->second->second.userD->AddTraffStatD(pi.first->second->second.dirD,
                                                          pi.first->second->first.GetSrcIP(),
                                                          pi.first->second->first.GetSrcPort(),
                                                          pi.first->second->second.lenD);
            #else
            pi.first->second->second.userD->AddTraffStatD(pi.first->second->second.dirD,
                                                          pi.first->second->first.GetSrcIP(),
                                                          pi.first->second->second.lenD);
            #endif
            }

        pi.first->second->second.userDPresent = false;
        }

    ++pi.first;
    }

ip2packets.erase(pi.first, pi.second);
}
//-----------------------------------------------------------------------------
void TRAFFCOUNTER_IMPL::SetUserNotifiers(USER_PTR user)
{
// Adding user. Adding notifiers to user.
TRF_IP_BEFORE ipBNotifier(*this, user);
ipBeforeNotifiers.push_front(ipBNotifier);
user->AddCurrIPBeforeNotifier(&(*ipBeforeNotifiers.begin()));

TRF_IP_AFTER ipANotifier(*this, user);
ipAfterNotifiers.push_front(ipANotifier);
user->AddCurrIPAfterNotifier(&(*ipAfterNotifiers.begin()));
}
//-----------------------------------------------------------------------------
void TRAFFCOUNTER_IMPL::UnSetUserNotifiers(USER_PTR user)
{
// Removing user. Removing notifiers from user.
std::list<TRF_IP_BEFORE>::iterator bi;
std::list<TRF_IP_AFTER>::iterator ai;

bi = ipBeforeNotifiers.begin();
while (bi != ipBeforeNotifiers.end())
    {
    if (user->GetLogin() == bi->GetUser()->GetLogin())
        {
        user->DelCurrIPBeforeNotifier(&(*bi));
        ipBeforeNotifiers.erase(bi);
        break;
        }
    ++bi;
    }

ai = ipAfterNotifiers.begin();
while (ai != ipAfterNotifiers.end())
    {
    if (user->GetLogin() == ai->GetUser()->GetLogin())
        {
        user->DelCurrIPAfterNotifier(&(*ai));
        ipAfterNotifiers.erase(ai);
        break;
        }
    ++ai;
    }
}
//-----------------------------------------------------------------------------
void TRAFFCOUNTER_IMPL::DeterminateDir(const RAW_PACKET & packet,
                                       int * dirU, // Direction for incoming packet
                                       int * dirD) const // Direction for outgoing packet
{
bool addrMatchU;
bool portMatchU;
bool addrMatchD;
bool portMatchD;
bool foundU = false; // Was rule for U found ?
bool foundD = false; // Was rule for D found ?
//printfd(__FILE__, "foundU=%d, foundD=%d\n", foundU, foundD);

enum { ICMP_RPOTO = 1, TCP_PROTO = 6, UDP_PROTO = 17 };

std::list<RULE>::const_iterator ln;
ln = rules.begin();

while (ln != rules.end())
    {
    if (!foundU)
        {
        addrMatchU = false;
        portMatchU = false;

        switch (ln->proto)
            {
            case all:
                portMatchU = true;
                break;

            case icmp:
                portMatchU = (packet.GetProto() == ICMP_RPOTO);
                break;

            case tcp_udp:
                if (packet.GetProto() == TCP_PROTO || packet.GetProto() == UDP_PROTO)
                    portMatchU = (packet.GetDstPort() >= ln->port1) && (packet.GetDstPort() <= ln->port2);
                break;

            case tcp:
                if (packet.GetProto() == TCP_PROTO)
                    portMatchU = (packet.GetDstPort() >= ln->port1) && (packet.GetDstPort() <= ln->port2);
                break;

            case udp:
                if (packet.GetProto() == UDP_PROTO)
                    portMatchU = (packet.GetDstPort() >= ln->port1) && (packet.GetDstPort() <= ln->port2);
                break;

            default:
                printfd(__FILE__, "Error! Incorrect rule!\n");
                break;
            }

        addrMatchU = (packet.GetDstIP() & ln->mask) == ln->ip;

        if (!foundU && addrMatchU && portMatchU)
            {
            foundU = true;
            *dirU = ln->dir;
            //printfd(__FILE__, "Up rule ok! %d\n", ln->dir);
            //PrintRule(ln->rule);
            }

        } //if (!foundU)

    if (!foundD)
        {
        addrMatchD = false;
        portMatchD = false;

        switch (ln->proto)
            {
            case all:
                portMatchD = true;
                break;

            case icmp:
                portMatchD = (packet.GetProto() == ICMP_RPOTO);
                break;

            case tcp_udp:
                if (packet.GetProto() == TCP_PROTO || packet.GetProto() == UDP_PROTO)
                    portMatchD = (packet.GetSrcPort() >= ln->port1) && (packet.GetSrcPort() <= ln->port2);
                break;

            case tcp:
                if (packet.GetProto() == TCP_PROTO)
                    portMatchD = (packet.GetSrcPort() >= ln->port1) && (packet.GetSrcPort() <= ln->port2);
                break;

            case udp:
                if (packet.GetProto() == UDP_PROTO)
                    portMatchD = (packet.GetSrcPort() >= ln->port1) && (packet.GetSrcPort() <= ln->port2);
                break;

            default:
                printfd(__FILE__, "Error! Incorrect rule!\n");
                break;
            }

        addrMatchD = (packet.GetSrcIP() & ln->mask) == ln->ip;

        if (!foundD && addrMatchD && portMatchD)
            {
            foundD = true;
            *dirD = ln->dir;
            //printfd(__FILE__, "Down rule ok! %d\n", ln->dir);
            //PrintRule(ln->rule);
            }
        } //if (!foundD)

    ++ln;
    }   //while (ln != rules.end())

if (!foundU)
    *dirU = DIR_NUM;

if (!foundD)
    *dirD = DIR_NUM;

return;
};
//-----------------------------------------------------------------------------
void TRAFFCOUNTER_IMPL::SetRulesFile(const std::string & fn)
{
rulesFileName = fn;
}
//-----------------------------------------------------------------------------
bool TRAFFCOUNTER_IMPL::ReadRules(bool test)
{
//printfd(__FILE__, "TRAFFCOUNTER::ReadRules()\n");

RULE rul;
FILE * f;
char str[1024];
char tp[100];   // protocol
char ta[100];   // address
char td[100];   // target direction
int r;
int lineNumber = 0;
f = fopen(rulesFileName.c_str(), "rt");

if (!f)
    {
    WriteServLog("File %s cannot be oppened.", rulesFileName.c_str());
    return true;
    }

while (fgets(str, 1023, f))
    {
    lineNumber++;
    if (str[strspn(str," \t")] == '#' || str[strspn(str," \t")] == '\n')
        {
        continue;
        }

    r = sscanf(str,"%s %s %s", tp, ta, td);
    if (r != 3)
        {
        WriteServLog("Error in file %s. There must be 3 parameters. Line %d.", rulesFileName.c_str(), lineNumber);
        fclose(f);
        return true;
        }

    rul.proto = 0xff;
    rul.dir = 0xff;

    for (int i = 0; i < PROTOMAX; i++)
        {
        if (strcasecmp(tp, protoName[i]) == 0)
            rul.proto = i;
        }

    for (int i = 0; i < DIR_NUM + 1; i++)
        {
        if (td == dirName[i])
            rul.dir = i;
        }

    if (rul.dir == 0xff || rul.proto == 0xff)
        {
        WriteServLog("Error in file %s. Line %d.",
                     rulesFileName.c_str(), lineNumber);
        fclose(f);
        return true;
        }

    if (ParseAddress(ta, &rul) != 0)
        {
        WriteServLog("Error in file %s. Error in adress. Line %d.",
                     rulesFileName.c_str(), lineNumber);
        fclose(f);
        return true;
        }
    if (!test)
        rules.push_back(rul);
    //PrintRule(rul);
    }

fclose(f);

// Adding lastest rule: ALL 0.0.0.0/0 NULL
rul.dir = DIR_NUM; //NULL
rul.ip = 0;  //0.0.0.0
rul.mask = 0;
rul.port1 = 0;
rul.port2 = 65535;
rul.proto = all;

if (!test)
    rules.push_back(rul);

//PrintRule(rul);

return false;
}
//-----------------------------------------------------------------------------
int TRAFFCOUNTER_IMPL::Reload()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (ReadRules(true))
    {
    WriteServLog("TRAFFCOUNTER: Cannot reload rules. Errors found.");
    return -1;
    }

FreeRules();
ReadRules();
WriteServLog("TRAFFCOUNTER: Reload rules successfull.");
return 0;
}
//-----------------------------------------------------------------------------
bool TRAFFCOUNTER_IMPL::ParseAddress(const char * ta, RULE * rule) const
{
char addr[50], mask[20], port1[20], port2[20], ports[40];

int len = strlen(ta);
char n = 0;
int i, p;
memset(addr, 0, sizeof(addr));
for (i = 0; i < len; i++)
    {
    if (ta[i] == '/' || ta[i] == ':')
        {
        addr[i] = 0;
        n = ta[i];
        break;
        }
    addr[i] = ta[i];
    n = 0;
    }
addr[i + 1] = 0;
p = i + 1;

if (n == '/')
    {
    // mask
    for (; i < len; i++)
        {
        if (ta[i] == ':')
            {
            mask[i - p] = 0;
            n = ':';
            break;
            }
        mask[i - p] = ta[i];
        }
    mask[i - p] = 0;
    }
else
    {
    strcpy(mask, "32");
    }

p = i + 1;
i++;

if (n == ':')
    {
    // port
    if (!(rule->proto == tcp || rule->proto == udp || rule->proto == tcp_udp))
        {
        WriteServLog("No ports specified for this protocol.");
        return true;
        }

    for (; i < len; i++)
        ports[i - p] = ta[i];

    ports[i - p] = 0;
    }
else
    {
    strcpy(ports, "0-65535");
    }

char *sss;
char pts[100];
strcpy(pts, ports);

if ((sss = strchr(ports, '-')) != NULL)
    {
    strncpy(port1, ports, int(sss-ports));
    port1[int(sss - ports)] = 0;
    strcpy(port2, sss + 1);
    }
else
    {
    strcpy(port1, ports);
    strcpy(port2, ports);
    }

// Convert strings to mask, ports and IP
int prt1, prt2, msk;
unsigned ip;
char *res;

msk = strtol(mask, &res, 10);
if (*res != 0)
    return true;

prt1 = strtol(port1, &res, 10);
if (*res != 0)
    return true;

prt2 = strtol(port2, &res, 10);
if (*res != 0)
    return true;

int r = inet_aton(addr, (struct in_addr*)&ip);
if (r == 0)
    return true;

rule->ip = ip;
rule->mask = CalcMask(msk);
//msk = 1;
//printfd(__FILE__, "msk=%d mask=%08X   mask=%08X\n", msk, rule->mask, (0xFFffFFff << (32 - msk)));

if ((ip & rule->mask) != ip)
    {
    WriteServLog("Address does'n match mask.");
    return true;
    }

rule->port1 = prt1;
rule->port2 = prt2;

return false;
}
//-----------------------------------------------------------------------------
uint32_t TRAFFCOUNTER_IMPL::CalcMask(uint32_t msk) const
{
if (msk >= 32) return 0xFFffFFff;
if (msk == 0) return 0;
return htonl(0xFFffFFff << (32 - msk));
}
//---------------------------------------------------------------------------
void TRAFFCOUNTER_IMPL::FreeRules()
{
rules.clear();
}
//-----------------------------------------------------------------------------
void TRAFFCOUNTER_IMPL::PrintRule(RULE rule) const
{
printf("%15s   ", inet_ntostring(rule.ip).c_str());
printf("mask=%08X ", rule.mask);
printf("port1=%5d ", rule.port1);
printf("port2=%5d ", rule.port2);
switch (rule.proto)
    {
    case 0:
        printf("TCP     ");
        break;
    case 1:
        printf("UDP     ");
        break;
    case 2:
        printf("ICMP    ");
        break;
    case 3:
        printf("TCP_UDP ");
        break;
    case 4:
        printf("ALL     ");
        break;
    }
printf("dir=%d \n", rule.dir);
return;
}
//-----------------------------------------------------------------------------
void TRAFFCOUNTER_IMPL::SetMonitorDir(const std::string & monitorDir)
{
TRAFFCOUNTER_IMPL::monitorDir = monitorDir;
monitoring = (monitorDir != "");
}
//-----------------------------------------------------------------------------
