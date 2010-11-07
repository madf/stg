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
 $Revision: 1.3 $
 $Date: 2009/04/10 14:15:46 $
 $Author: faust $
 */


#ifndef TRAFFCOUNTER_H
#define TRAFFCOUNTER_H

#include <pthread.h>
#include <netinet/ip.h>

#ifdef HAVE_STDINT
    #include <stdint.h>
#else
    #ifdef HAVE_INTTYPES
        #include <inttypes.h>
    #else
        #error "You need either stdint.h or inttypes.h to compile this!"
    #endif
#endif

#include <ctime>
#include <list>
#include <vector>
#include <map>

#include "rules.h"
#include "rules_finder.h"
#include "tc_packets.h"
#include "user_tc_iface.h"
#include "capturer_tc_iface.h"

#define PACKET_TIMEOUT 300

namespace STG
{

    class TRAFFCOUNTER : public IUSER_TC, public ICAPTURER_TC
    {
    public:
        TRAFFCOUNTER();
        ~TRAFFCOUNTER();

        void            SetRules(const RULES & data);

        bool            Start();
        bool            Stop();

        // Capturer API
        void            AddPacket(const iphdr & ipHdr, uint16_t sport, uint16_t dport);

        // User API
        void            AddIP(uint32_t ip);
        void            DeleteIP(uint32_t ip, TRAFF_DATA * traff);
        void            GetIP(uint32_t ip, TRAFF_DATA * traff);

        /*
         * Stream quality represents a "scatterness" of data stream
         * When sessions represend a large amount of information - it's a good
         * stream. Most of common-use protocols (HTTP, FTP, etc.) shows a good
         * stream quality.
         * When there are a lot of packet that creates a new streams - it's a
         * bad stream. p2p traffic has a bias to show a bad stream quality.
         */
        double          StreamQuality() const;
        uint64_t        PendingCount() const { return pendingCount; };
        uint64_t        SessionsCount() const { return sessions.size(); };
        uint64_t        IndexesCount() const { return ip2sessions.size(); };
        uint64_t        CacheHits() const { return cacheHits; };
        uint64_t        CacheMisses() const { return cacheMisses; };

    private:
        static void *   Run(void * data);

        void            Process(const PENDING_PACKET & p);

        RULES_FINDER rulesFinder;

        /*
         * SESSION_INDEX: ip -> SESSION_ITER
         * SESSIONS: SESSION_ID -> SESSION_DATA
         *                -> SESSION_INDEX (saddr)
         *                -> SESSION_INDEX (daddr)
         */
        struct SESSION_FULL_DATA; // Forward declaration
        typedef std::map<SESSION_ID, SESSION_FULL_DATA, SESSION_LESS> SESSIONS;
        typedef SESSIONS::iterator SESSION_ITER;
        /*
         *  This structure is used to take a fast session access by IP
         * Normally, one IP can reffer multiple sessions. For each data stream there
         * are 2 sessions: incoming data and outgoing data.
         */
        typedef std::multimap<uint32_t, SESSION_ITER> SESSION_INDEX;
        typedef SESSION_INDEX::iterator INDEX_ITER;
        /*
         *  Append session meta-information with back-indexes
         * In process of removing IP from TRAFFCOUNTER we need to remove indexes of
         * sessions, reffered by this IP. To prevent slow searching by index tree we
         * use 2 back-references: for source and destination IP.
         */
        struct SESSION_FULL_DATA : public SESSION_DATA
        {
            INDEX_ITER sIdx; // Back reference for fast index removing
            INDEX_ITER dIdx; // Back reference for fast index removing
            int        refCount; // Reference count for packet removing
        };

        std::list<PENDING_PACKET> pendingPackets;

        SESSIONS sessions; // A map with sessions data

        /*
         * When pending packet appends a session - it's a "cache hit"
         * When pending packet creates a new session - it's a "cache miss"
         */
        uint64_t cacheHits;
        uint64_t cacheMisses;
        uint64_t pendingCount;
        uint64_t maxPending;

        SESSION_INDEX ip2sessions; // IP index for sessions data

        /*
         * A sorted vector of allowed/disallowed ips
         */
        std::vector<uint32_t> ips;
        typedef std::vector<uint32_t>::iterator IP_ITER;

        bool        stopped;
        bool        running;

        mutable pthread_mutex_t         sessionMutex; // For sessions
        mutable pthread_mutex_t         pendingMutex; // For pendinPackets
        mutable pthread_cond_t          pendingCond;  //
        mutable pthread_mutex_t         ipMutex;      // For ip list
        mutable pthread_mutex_t         rulesMutex;   // For rules list
        pthread_t               thread;

    };

}

#endif //TRAFFCOUNTER_H
