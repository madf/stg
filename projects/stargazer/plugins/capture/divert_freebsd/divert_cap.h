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
 Author : Boris Mikhailenko <stg34@stg.dp.ua>
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
class DIVERT_CAP : public STG::Plugin {
public:
    DIVERT_CAP();

    void                SetTraffcounter(STG::TraffCounter * tc) override { traffCnt = tc; }

    int                 Start() override;
    int                 Stop() override;
    int                 Reload(const STG::ModuleSettings & /*ms*/) override { return 0; }
    bool                IsRunning() override { return isRunning; }

    void                SetSettings(const STG::ModuleSettings & s) override { settings = s; }
    int                 ParseSettings() override;
    const std::string & GetStrError() const override { return errorStr; }
    std::string         GetVersion() const override;
    uint16_t            GetStartPosition() const override { return 40; }
    uint16_t            GetStopPosition() const override { return 40; }

private:
    DIVERT_CAP(const DIVERT_CAP & rvalue);
    DIVERT_CAP & operator=(const DIVERT_CAP & rvalue);

    void                Run(std::stop_token token) noexcept;

    int                 DivertCapOpen();
    int                 DivertCapOpen(int n);
    int                 DivertCapRead(char * buffer, int blen, char ** iface);
    int                 DivertCapRead(char * buffer, int blen, char ** iface, int n);
    int                 DivertCapClose();

    STG::ModuleSettings     settings;

    int                 port;
    bool                disableForwarding;

    mutable std::string errorStr;

    std::jthread        m_thread;

    bool                isRunning;

    STG::TraffCounter *      traffCnt;

    STG::PluginLogger       logger;
};
