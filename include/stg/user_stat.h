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

#include "stg/optional.h"
#include "user_traff.h"

#include <ctime>
#include <cstdint>
#include <map>
#include <utility>
#include <string>

namespace STG
{
//-----------------------------------------------------------------------------
struct IPDirPair
{
    #ifdef TRAFF_STAT_WITH_PORTS
    IPDirPair(uint32_t _ip, int _dir, uint16_t _port) noexcept
        : ip(_ip),
          dir(_dir),
          port(_port)
    {}
    #else
    IPDirPair(uint32_t _ip, int _dir) noexcept
        : ip(_ip),
          dir(_dir)
    {}
    #endif
    //------------------------
    bool operator<(const IPDirPair& rhs) const noexcept
    {
        if (ip < rhs.ip)
            return true;

        if (ip > rhs.ip)
            return false;

        #ifdef TRAFF_STAT_WITH_PORTS
        if (port < rhs.port)
            return true;

        if (port > rhs.port)
            return false;
        #endif

        if (dir < rhs.dir)
            return true;

        return false;
    }
    //------------------------
    bool operator==(const IPDirPair& rhs) const noexcept
    {
        #ifdef TRAFF_STAT_WITH_PORTS
        return ip == rhs.ip && port == rhs.port && dir == rhs.dir;
        #else
        return ip == rhs.ip && dir == rhs.dir;
        #endif
    }
    bool operator!=(const IPDirPair& rhs) const noexcept
    {
        return !operator==(rhs);
    }

    IPDirPair(const IPDirPair&) = default;
    IPDirPair& operator=(const IPDirPair&) = default;
    IPDirPair(IPDirPair&&) = default;
    IPDirPair& operator=(IPDirPair&&) = default;
    //------------------------
    uint32_t ip;
    int      dir;
    #ifdef TRAFF_STAT_WITH_PORTS
    uint16_t port;
    #endif
};
//-----------------------------------------------------------------------------
struct StatNode
{
    StatNode(uint64_t _up, uint64_t _down, double _cash) noexcept
        : up(_up),
          down(_down),
          cash(_cash)
    {}

    StatNode(const StatNode&) = default;
    StatNode& operator=(const StatNode&) = default;
    StatNode(StatNode&&) = default;
    StatNode& operator=(StatNode&&) = default;

    uint64_t up;
    uint64_t down;
    double cash;
};
//-----------------------------------------------------------------------------
struct UserStat
{
    UserStat() noexcept
        : cash(0),
          freeMb(0),
          lastCashAdd(0),
          lastCashAddTime(0),
          passiveTime(0),
          lastActivityTime(0)
    {}

    UserStat(const UserStat&) = default;
    UserStat& operator=(const UserStat&) = default;
    UserStat(UserStat&&) = default;
    UserStat& operator=(UserStat&&) = default;

    DirTraff sessionUp;
    DirTraff sessionDown;
    DirTraff monthUp;
    DirTraff monthDown;
    double   cash;
    double   freeMb;
    double   lastCashAdd;
    time_t   lastCashAddTime;
    time_t   passiveTime;
    time_t   lastActivityTime;
};
//-----------------------------------------------------------------------------
using TraffStat = std::map<IPDirPair, StatNode>;
//-----------------------------------------------------------------------------
using CashInfo = std::pair<double, std::string>;
//-----------------------------------------------------------------------------
struct UserStatOpt
{
    UserStatOpt() = default;

    UserStatOpt(const UserStat& data) noexcept
        : cash(data.cash),
          freeMb(data.freeMb),
          lastCashAdd(data.lastCashAdd),
          lastCashAddTime(data.lastCashAddTime),
          passiveTime(data.passiveTime),
          lastActivityTime(data.lastActivityTime),
          sessionUp(data.sessionUp),
          sessionDown(data.sessionDown),
          monthUp(data.monthUp),
          monthDown(data.monthDown)
    {}
    UserStatOpt& operator=(const UserStat& us)
    {
        cash             = us.cash;
        freeMb           = us.freeMb;
        lastCashAdd      = us.lastCashAdd;
        lastCashAddTime  = us.lastCashAddTime;
        passiveTime      = us.passiveTime;
        lastActivityTime = us.lastActivityTime;
        sessionUp        = us.sessionUp;
        sessionDown      = us.sessionDown;
        monthUp          = us.monthUp;
        monthDown        = us.monthDown;
        return *this;
    }

    UserStatOpt(const UserStatOpt&) = default;
    UserStatOpt& operator=(const UserStatOpt&) = default;
    UserStatOpt(UserStatOpt&&) = default;
    UserStatOpt& operator=(UserStatOpt&&) = default;

    Optional<double>    cash;
    Optional<CashInfo>  cashAdd;
    Optional<CashInfo>  cashSet;
    Optional<double>    freeMb;
    Optional<double>    lastCashAdd;
    Optional<time_t>    lastCashAddTime;
    Optional<time_t>    passiveTime;
    Optional<time_t>    lastActivityTime;
    DirTraffOpt         sessionUp;
    DirTraffOpt         sessionDown;
    DirTraffOpt         monthUp;
    DirTraffOpt         monthDown;
};
//-----------------------------------------------------------------------------
}
