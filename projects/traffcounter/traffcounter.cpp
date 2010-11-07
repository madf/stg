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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

/*
 $Revision: 1.5 $
 $Date: 2009/10/12 08:43:32 $
 $Author: faust $
 */

#include <csignal>
#include <cassert>
#include <algorithm>

#include "traffcounter.h"
#include "logger.h"
#include "lock.h"
#include "utils.h"

//-----------------------------------------------------------------------------
STG::TRAFFCOUNTER::TRAFFCOUNTER()
    : rulesFinder(),
      pendingPackets(),
      sessions(),
      cacheHits(0),
      cacheMisses(0),
      pendingCount(0),
      maxPending(0),
      ip2sessions(),
      stopped(true),
      running(false)
{
LOG_IT << "TRAFFCOUNTER::TRAFFCOUNTER()\n";
pthread_mutex_init(&sessionMutex, NULL);
pthread_mutex_init(&pendingMutex, NULL);
pthread_mutex_init(&ipMutex, NULL);
pthread_mutex_init(&rulesMutex, NULL);
pthread_cond_init(&pendingCond, NULL);
}
//-----------------------------------------------------------------------------
STG::TRAFFCOUNTER::~TRAFFCOUNTER()
{
LOG_IT << "TRAFFCOUNTER::~TRAFFCOUNTER()\n";
pthread_cond_destroy(&pendingCond);
pthread_mutex_destroy(&rulesMutex);
pthread_mutex_destroy(&ipMutex);
pthread_mutex_destroy(&pendingMutex);
pthread_mutex_destroy(&sessionMutex);
}
//-----------------------------------------------------------------------------
//  Starting processing thread
bool STG::TRAFFCOUNTER::Start()
{
LOG_IT << "TRAFFCOUNTER::Start()\n";

if (running)
    return false;

running = true;
stopped = true;

if (pthread_create(&thread, NULL, Run, this))
    {
    LOG_IT << "TRAFFCOUNTER::Start() Error: Cannot start thread!\n";
    return true;
    }

return false;
}
//-----------------------------------------------------------------------------
bool STG::TRAFFCOUNTER::Stop()
{
LOG_IT << "TRAFFCOUNTER::Stop()\n";
LOG_IT << "maxPending: " << maxPending << std::endl;

if (!running)
    return false;

running = false;
// Awake thread
pthread_cond_signal(&pendingCond);

//5 seconds to thread stops itself
for (int i = 0; i < 25 && !stopped; ++i)
    {
    usleep(200000);
    }

//after 5 seconds waiting thread still running. now kill it
if (!stopped)
    {
    LOG_IT << "TRAFFCOUNTER::Stop() Killing thread\n";
    if (pthread_kill(thread, SIGINT))
        {
        return true;
        }
    LOG_IT << "TRAFFCOUNTER::Stop() Thread killed\n";
    }

return false;
}
//-----------------------------------------------------------------------------
double STG::TRAFFCOUNTER::StreamQuality() const
{
if (!cacheHits && !cacheMisses)
    {
    return 0;
    }

double quality = cacheHits;
return quality / (quality + cacheMisses);
}
//-----------------------------------------------------------------------------
void STG::TRAFFCOUNTER::AddPacket(const iphdr & ipHdr, uint16_t sport, uint16_t dport)
{
/*
 *  Intersects with AddIP (from user thread), DeleteIP (from user thread) and
 * Process (internal thread). AddPacket is calling from capturer's thread
 *
 *  ips is affected by AddIP (logarithmic lock time) and
 *  DeleteIP (from user thread)
 *
 *  May be locked by AddIP or DeleteIP (from user thread)
 *
 *  Lock AddIP (user thread) or DeleteIP (user thread)
 *  Logarithmic lock time
 */

bool srcExists;
bool dstExists;

    {
    SCOPED_LOCK lock(ipMutex);
    srcExists = std::binary_search(ips.begin(), ips.end(), ipHdr.saddr);
    dstExists = std::binary_search(ips.begin(), ips.end(), ipHdr.daddr);
    }

if (!srcExists &&
    !dstExists)
    {
    // Just drop the packet
    return;
    }

STG::PENDING_PACKET p(ipHdr, sport, dport);

// Packet classification
if (srcExists)
    {
    if (dstExists)
        {
        // Both src and dst are countable
        p.direction = PENDING_PACKET::LOCAL;
        }
    else
        {
        // Src is countable
        p.direction = PENDING_PACKET::OUTGOING;
        }
    }
else
    {
    if (dstExists)
        {
        // Dst is countable
        p.direction = PENDING_PACKET::INCOMING;
        }
    else
        {
        assert(0);
        // Not src nor dst are countable
        p.direction = PENDING_PACKET::FOREIGN;
        }
    }

/*
 *  pendingPackets is affected by Process (from internal thread)
 *
 *  May be locked by Process (internal thread)
 *
 *  Lock Process (internal thread)
 *  Constant lock time
 */
SCOPED_LOCK lock(pendingMutex);
pendingPackets.push_back(p);
pendingCount++;
#ifdef STATISTIC
if (pendingCount > maxPending)
    maxPending = pendingCount;
#endif
pthread_cond_signal(&pendingCond);

}
//-----------------------------------------------------------------------------
void STG::TRAFFCOUNTER::AddIP(uint32_t ip)
{
/*
 *  AddIP is calling from users and affect DeleteIP and AddPacket.
 * DeleteIP cannot be called concurrently with AddIP - it's the same
 * thread. AddPacket is calling from capturer's thread - concurrently
 * with AddIP.
 *
 * May be locked by AddPacket (from capturer's thread)
 * Logarithmic lock time
 *
 * Lock AddPacket (capturer's thread)
 * Logarithmic lock time
 */
SCOPED_LOCK lock(ipMutex);
IP_ITER it(std::lower_bound(ips.begin(), ips.end(), ip));

if (it != ips.end() && *it == ip)
    {
    return;
    }
// Insertion
ips.insert(it, ip);
}
//-----------------------------------------------------------------------------
void STG::TRAFFCOUNTER::DeleteIP(uint32_t ip, STG::TRAFF_DATA * traff)
{
/*
 *  DeleteIP is calling from users and affect AddIP, AddPacket, GetIP and
 * Process. AddIP and GetIP cannot be called concurrently with DeleteIP - it's
 * the same thread. AddPacket is calling from capturer's thread - concurrently
 * with DeleteIP. Process is calling from internal thread - concurrently with
 * DeleteIP.
 *
 * May be locked by AddPacket (from capturer's thread)
 * Logarithmic lock time
 *
 * Lock AddPacket (capturer's thread)
 * Logarithmic lock time
 */

    {
    SCOPED_LOCK lock(ipMutex);

    IP_ITER it(std::lower_bound(ips.begin(), ips.end(), ip));
    if (it == ips.end())
        {
        return;
        }
    if (*it != ip)
        {
        return;
        }

    ips.erase(it);
    }

// Get sessions for this ip
std::pair<INDEX_ITER,
          INDEX_ITER> range;

SCOPED_LOCK lock(sessionMutex);
range = ip2sessions.equal_range(ip);
std::list<INDEX_ITER> toDelete;

// Lock session growing
for (INDEX_ITER it = range.first; it != range.second; ++it)
    {
    traff->push_back(STG::TRAFF_ITEM(it->second->first, it->second->second));

    // Include self
    toDelete.push_back(it);

    /*if (ip == it->second->first.saddr)
        {
        toDelete.push_back(it->second->second.dIdx);
        }
    else
        {
        toDelete.push_back(it->second->second.sIdx);
        }*/

    --it->second->second.refCount;

    // Remove session
    /*
     *  Normally we will lock here only in case of session between
     *  two users from ips list
     */
    if (!it->second->second.refCount)
        {
        sessions.erase(it->second);
        }
    }

// Remove indexes
for (std::list<INDEX_ITER>::iterator it = toDelete.begin();
     it != toDelete.end();
     ++it)
    {
    ip2sessions.erase(*it);
    }
}
//-----------------------------------------------------------------------------
void STG::TRAFFCOUNTER::GetIP(uint32_t ip, STG::TRAFF_DATA * traff)
{
/*
 *  Normally we will lock here only in case of session between
 *  two users from ips list
 */
std::pair<INDEX_ITER,
          INDEX_ITER> range;

SCOPED_LOCK lock(sessionMutex);
range = ip2sessions.equal_range(ip);
std::list<INDEX_ITER> toDelete;

// TODO: replace with foreach
for (SESSION_INDEX::iterator it = range.first;
     it != range.second;
     ++it)
    {
    traff->push_back(STG::TRAFF_ITEM(it->second->first, it->second->second));
    toDelete.push_back(it);
    --it->second->second.refCount;
    if (!it->second->second.refCount)
        {
        sessions.erase(it->second);
        }
    }

for (std::list<INDEX_ITER>::iterator it = toDelete.begin();
     it != toDelete.end();
     ++it)
    {
    ip2sessions.erase(*it);
    }
}
//-----------------------------------------------------------------------------
void * STG::TRAFFCOUNTER::Run(void * data)
{
STG::TRAFFCOUNTER * tc = static_cast<STG::TRAFFCOUNTER *>(data);
tc->stopped = false;

while (tc->running)
    {
    STG::PENDING_PACKET packet;
        {
        SCOPED_LOCK lock(tc->pendingMutex);
        if (tc->pendingPackets.empty())
            {
            pthread_cond_wait(&tc->pendingCond, &tc->pendingMutex);
            }
        if (!tc->running)
            {
            break;
            }
        packet = *tc->pendingPackets.begin();
        tc->pendingPackets.pop_front();
        --tc->pendingCount;
        }
    tc->Process(packet);
    }

tc->stopped = true;
return NULL;
}
//-----------------------------------------------------------------------------
void STG::TRAFFCOUNTER::Process(const STG::PENDING_PACKET & p)
{
// Bypass on stop
if (!running)
    return;

// Fail on foreign packets
if (p.direction == PENDING_PACKET::FOREIGN) {
    assert(0);
}

// Searching a new packet in a tree.
SESSION_ITER si;
    {
    SCOPED_LOCK lock(sessionMutex);
    si = sessions.find(STG::SESSION_ID(p));
    }

// Packet found - update length and time
if (si != sessions.end())
    {
    // Grow session
    SCOPED_LOCK lock(sessionMutex);
    si->second.length += p.length;
    ++cacheHits;
    return;
    }

++cacheMisses;

// Packet not found - add new packet

// This packet is alowed to create session
STG::SESSION_ID sid(p);
SESSION_FULL_DATA sd;

// Identify a packet
    {
    SCOPED_LOCK lock(rulesMutex);
    sd.dir = rulesFinder.GetDir(p);
    }

sd.length = p.length;

if (p.direction == PENDING_PACKET::LOCAL)
    {
    sd.refCount = 2;
    }
else
    {
    sd.refCount = 1;
    }

// Create a session
std::pair<SESSION_ITER,
          bool> sIt(sessions.insert(std::make_pair(sid, sd)));
    {
    SCOPED_LOCK lock(sessionMutex);
    std::pair<SESSION_ITER,
              bool> sIt(sessions.insert(std::make_pair(sid, sd)));

    // Create an indexes
    sIt.first->second.sIdx = ip2sessions.insert(std::make_pair(p.saddr, sIt.first));
    sIt.first->second.dIdx = ip2sessions.insert(std::make_pair(p.daddr, sIt.first));
    }

}
//-----------------------------------------------------------------------------
void STG::TRAFFCOUNTER::SetRules(const STG::RULES & data)
{
/*
 *  SetRules is calling from outside internel thread. Process is calling
 * from internal thread and calls DeterminateDir which use rules data.
 *
 * May be locked by DeterminateDir (Process) from internal thread.
 *
 * Lock DeterminateDir (Process) - internal thread.
 * Linear lock time
 */
SCOPED_LOCK lock(rulesMutex);
rulesFinder.SetRules(data);
}
