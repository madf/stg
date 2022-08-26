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
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

#pragma once

#include "stg/traffcounter.h"
#include "stg/logger.h"
#include "stg/raw_ip_packet.h"
#include "stg/subscriptions.h"
#include "user_impl.h"

#include <list>
#include <vector>
#include <tuple>
#include <map>
#include <string>
#include <mutex>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop
#include <cstdint>
#include <ctime>

#define PROTOMAX    (5)

namespace STG
{

class UsersImpl;

//-----------------------------------------------------------------------------
struct Rule {
uint32_t    ip;             // IP
uint32_t    mask;           // Network mask
uint16_t    port1;          // Min port
uint16_t    port2;          // Max port
uint8_t     proto;          // Protocol
uint32_t    dir;            // Direction
};
//-----------------------------------------------------------------------------
struct PacketExtraData {
    PacketExtraData()
        : flushTime(0),
          updateTime(0),
          userU(NULL),
          userUPresent(false),
          userD(NULL),
          userDPresent(false),
          dirU(DIR_NUM),
          dirD(DIR_NUM),
          lenU(0),
          lenD(0)
    {}

    time_t      flushTime;          // Last flush time
    time_t      updateTime;         // Last update time
    UserImpl * userU;              // Uploader
    bool        userUPresent;       // Uploader is registered user
    UserImpl * userD;              // Downloader
    bool        userDPresent;       // Downloader is registered user
    int         dirU;               // Upload direction
    int         dirD;               // Download direction
    uint32_t    lenU;               // Upload length
    uint32_t    lenD;               // Download length
};
//-----------------------------------------------------------------------------
class TraffCounterImpl : public TraffCounter {
    public:
        TraffCounterImpl(UsersImpl * users, const std::string & rulesFileName);
        ~TraffCounterImpl();

        int         Reload();
        int         Start();
        int         Stop();

        void        process(const RawPacket & rawPacket) override;
        void        SetMonitorDir(const std::string & monitorDir);

        size_t      rulesCount() const override { return rules.size(); }

    private:
        bool        ParseAddress(const char * ta, Rule * rule) const;
        uint32_t    CalcMask(uint32_t msk) const;
        void        FreeRules();
        bool        ReadRules(bool test = false);

        void        Run(std::stop_token token);

        void        DeterminateDir(const RawPacket & packet,
                                   int * dirU, // Direction for upload
                                   int * dirD) const; // Direction for download

        void        FlushAndRemove();

        void        AddUser(UserImpl * user);
        void        DelUser(uint32_t uip);
        void        SetUserNotifiers(UserImpl* user);
        void        UnSetUserNotifiers(UserImpl* user);

        typedef std::list<Rule>::iterator rule_iter;

        std::list<Rule>          rules;

        typedef std::map<RawPacket, PacketExtraData> Packets;
        typedef Packets::iterator pp_iter;
        typedef std::multimap<uint32_t, pp_iter> Index;
        typedef Index::iterator ip2p_iter;
        typedef Index::const_iterator ip2p_citer;

        Packets packets; // Packets tree

        Index ip2packets; // IP-to-Packet index

        std::string              dirName[DIR_NUM + 1];

        Logger &             WriteServLog;
        std::string              rulesFileName;

        std::string              monitorDir;
        bool                     monitoring;
        time_t                   touchTimeP;

        UsersImpl *             users;

        bool                     stopped;
        std::mutex               m_mutex;
        std::jthread             m_thread;

        ScopedConnection m_onAddUserConn;
        ScopedConnection m_onDelUserConn;

        using OnIPConns = std::tuple<int, ScopedConnection, ScopedConnection>;
        std::vector<OnIPConns> m_onIPConns;
        void beforeIPChange(uint32_t oldVal);
        void afterIPChange(UserImpl* user, uint32_t newVal);
};

}
//-----------------------------------------------------------------------------
