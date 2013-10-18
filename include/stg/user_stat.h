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

 /*
 $Revision: 1.15 $
 $Date: 2010/03/11 14:42:05 $
 $Author: faust $
 */

#ifndef USER_STAT_H
#define USER_STAT_H

#include <ctime>
#include <map>
#include <utility>
#include <string>

#include "os_int.h"
#include "resetable.h"
#include "user_traff.h"
//-----------------------------------------------------------------------------
struct IP_DIR_PAIR
{
    #ifdef TRAFF_STAT_WITH_PORTS
    IP_DIR_PAIR(uint32_t _ip,
                int _dir,
                uint16_t _port)
        : ip(_ip),
          dir(_dir),
          port(_port)
    {}
    #else
    IP_DIR_PAIR(uint32_t _ip,
                int _dir)
        : ip(_ip),
          dir(_dir)
    {}
    #endif
    //------------------------
    bool operator<(const IP_DIR_PAIR & idp) const
        {
        if (ip < idp.ip)
            return true;

        if (ip > idp.ip)
            return false;

        #ifdef TRAFF_STAT_WITH_PORTS
        if (port < idp.port)
            return true;

        if (port > idp.port)
            return false;
        #endif

        if (dir < idp.dir)
            return true;

        return false;
        }
    //------------------------
    bool operator!=(const IP_DIR_PAIR & rvalue) const
        {
        if (ip != rvalue.ip)
            return true;

        #ifdef TRAFF_STAT_WITH_PORTS
        if (port != rvalue.port)
            return true;
        #endif

        if (dir != rvalue.dir)
            return true;

        return false;
        }
    //------------------------
    uint32_t        ip;
    int             dir;
    #ifdef TRAFF_STAT_WITH_PORTS
    uint16_t        port;
    #endif
};
//-----------------------------------------------------------------------------
struct STAT_NODE
{
    STAT_NODE(uint64_t _up,
              uint64_t _down,
              double   _cash)
        : up(_up),
          down(_down),
          cash(_cash)
    {}
    uint64_t        up;
    uint64_t        down;
    double          cash;
};
//-----------------------------------------------------------------------------
struct USER_STAT
{
    USER_STAT()
        : sessionUp(),
          sessionDown(),
          monthUp(),
          monthDown(),
          cash(0),
          freeMb(0),
          lastCashAdd(0),
          lastCashAddTime(0),
          passiveTime(0),
          lastActivityTime(0)
    {}

    DIR_TRAFF   sessionUp;
    DIR_TRAFF   sessionDown;
    DIR_TRAFF   monthUp;
    DIR_TRAFF   monthDown;
    double      cash;
    double      freeMb;
    double      lastCashAdd;
    time_t      lastCashAddTime;
    time_t      passiveTime;
    time_t      lastActivityTime;
};
//-----------------------------------------------------------------------------
typedef std::map<IP_DIR_PAIR, STAT_NODE> TRAFF_STAT;
//-----------------------------------------------------------------------------
typedef std::pair<double, std::string> CASH_INFO;
//-----------------------------------------------------------------------------
struct USER_STAT_RES
{
    USER_STAT_RES()
        : cash(),
          freeMb(),
          lastCashAdd(),
          lastCashAddTime(),
          passiveTime(),
          lastActivityTime(),
          sessionUp(),
          sessionDown(),
          monthUp(),
          monthDown()
    {}

    USER_STAT_RES & operator= (const USER_STAT & us)
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
    USER_STAT GetData() const
    {
        USER_STAT us;
        us.cash             = cash.data();
        us.freeMb           = freeMb.data();
        us.lastCashAdd      = lastCashAdd.data();
        us.lastCashAddTime  = lastCashAddTime.data();
        us.passiveTime      = passiveTime.data();
        us.lastActivityTime = lastActivityTime.data();
        us.sessionUp        = sessionUp.GetData();
        us.sessionDown      = sessionDown.GetData();
        us.monthUp          = monthUp.GetData();
        us.monthDown        = monthDown.GetData();
        return us;
    }

    RESETABLE<double>      cash;
    RESETABLE<CASH_INFO>   cashAdd;
    RESETABLE<CASH_INFO>   cashSet;
    RESETABLE<double>      freeMb;
    RESETABLE<double>      lastCashAdd;
    RESETABLE<time_t>      lastCashAddTime;
    RESETABLE<time_t>      passiveTime;
    RESETABLE<time_t>      lastActivityTime;
    DIR_TRAFF_RES          sessionUp;
    DIR_TRAFF_RES          sessionDown;
    DIR_TRAFF_RES          monthUp;
    DIR_TRAFF_RES          monthDown;
};
//-----------------------------------------------------------------------------
#endif
