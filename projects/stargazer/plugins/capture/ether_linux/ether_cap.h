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
 $Revision: 1.12 $
 $Date: 2009/12/13 13:45:13 $
 */

/*
* Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
*/

#pragma once

#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/logger.h"

#include <string>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop

namespace STG
{

class Users;
class Tariffs;
struct Admins;
struct TraffCounter;
struct Settings;

}

//-----------------------------------------------------------------------------
class ETHER_CAP : public STG::Plugin {
public:
    ETHER_CAP();

    void                SetTraffcounter(STG::TraffCounter * tc) { traffCnt = tc; }

    int                 Start();
    int                 Stop();
    int                 Reload(const STG::ModuleSettings & /*ms*/) { return 0; }
    bool                IsRunning() { return isRunning; }

    int                 ParseSettings() { return 0; }
    const std::string & GetStrError() const { return errorStr; }
    std::string         GetVersion() const;
    uint16_t            GetStartPosition() const { return 40; }
    uint16_t            GetStopPosition() const { return 40; }

private:
    ETHER_CAP(const ETHER_CAP & rvalue);
    ETHER_CAP & operator=(const ETHER_CAP & rvalue);

    void                Run(std::stop_token token);
    int                 EthCapOpen();
    int                 EthCapClose();
    int                 EthCapRead(void * buffer, int blen, char ** iface);

    mutable std::string errorStr;

    std::jthread        m_thread;
    bool                isRunning;
    int                 capSock;

    STG::TraffCounter *      traffCnt;

    STG::PluginLogger       logger;
};
