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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#pragma once

#include "stg/plugin.h"

#include <string>
#include <stdexcept>
#include <cstdint>

namespace STG
{

struct ModuleSettings;
struct Settings;
struct Admins;
struct Tariffs;
struct Users;
struct Services;
struct Corporations;
struct TraffCounter;
struct Store;

//-----------------------------------------------------------------------------
class PluginRunner {
public:
    struct Error : public std::runtime_error {
        explicit Error(const std::string & msg) : runtime_error(msg) {}
    };

    PluginRunner(const std::string& pluginFileName,
                 const std::string& pluginName,
                 const ModuleSettings& ms,
                 Admins& admins,
                 Tariffs& tariffs,
                 Users& users,
                 Services& services,
                 Corporations& corporations,
                 TraffCounter& traffcounter,
                 Store& store,
                 const Settings & settings);
    ~PluginRunner();

    int             Start();
    int             Stop();
    int             Reload(const ModuleSettings& ms);
    int             Restart();
    bool            IsRunning() { return m_plugin.IsRunning(); }

    const std::string& GetStrError() const { return errorStr; }
    Plugin& GetPlugin() { return m_plugin; }
    const std::string& GetFileName() const { return pluginFileName; }
    const std::string& GetName() const { return pluginName; }

    uint16_t        GetStartPosition() const { return m_plugin.GetStartPosition(); }
    uint16_t        GetStopPosition() const { return m_plugin.GetStopPosition(); }

private:
    Plugin & load(const ModuleSettings& ms,
                  Admins& admins,
                  Tariffs& tariffs,
                  Users& users,
                  Services& services,
                  Corporations& corporations,
                  TraffCounter& traffcounter,
                  Store& store,
                  const Settings& settings);

    std::string pluginFileName;
    std::string pluginName;
    void*       libHandle;

    Plugin&     m_plugin;
    std::string errorStr;
};
//-----------------------------------------------------------------------------
}
