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
 *    Author : Maksym Mamontov <stg@madf.info>
 */

#ifndef PLUGIN_RUNNER_H
#define PLUGIN_RUNNER_H

#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/os_int.h"

#include <string>
#include <stdexcept>

class SETTINGS;
class ADMINS;
class TARIFFS;
class USERS;
class SERVICES;
class CORPORATIONS;
class TRAFFCOUNTER;
class STORE;

//-----------------------------------------------------------------------------
class PLUGIN_RUNNER {
public:
    struct Error : public std::runtime_error {
        explicit Error(const std::string & msg) : runtime_error(msg) {}
    };

    PLUGIN_RUNNER(const std::string & pluginFileName,
                  const std::string & pluginName,
                  const MODULE_SETTINGS & ms,
                  ADMINS & admins,
                  TARIFFS & tariffs,
                  USERS & users,
                  SERVICES & services,
                  CORPORATIONS & corporations,
                  TRAFFCOUNTER & traffcounter,
                  STORE & store,
                  const SETTINGS & settings);
    ~PLUGIN_RUNNER();

    int             Start();
    int             Stop();
    int             Reload(const MODULE_SETTINGS & ms);
    int             Restart();
    bool            IsRunning() { return m_plugin.IsRunning(); }

    const std::string & GetStrError() const { return errorStr; }
    PLUGIN &        GetPlugin() { return m_plugin; }
    const std::string & GetFileName() const { return pluginFileName; }
    const std::string & GetName() const { return pluginName; }

    uint16_t        GetStartPosition() const { return m_plugin.GetStartPosition(); }
    uint16_t        GetStopPosition() const { return m_plugin.GetStopPosition(); }

private:
    PLUGIN_RUNNER(const PLUGIN_RUNNER & rvalue);
    PLUGIN_RUNNER & operator=(const PLUGIN_RUNNER & rvalue);

    PLUGIN & Load(const MODULE_SETTINGS & ms,
                  ADMINS & admins,
                  TARIFFS & tariffs,
                  USERS & users,
                  SERVICES & services,
                  CORPORATIONS & corporations,
                  TRAFFCOUNTER & traffcounter,
                  STORE & store,
                  const SETTINGS & settings);

    std::string     pluginFileName;
    std::string     pluginName;
    void *          libHandle;

    PLUGIN &        m_plugin;
    std::string     errorStr;
};
//-----------------------------------------------------------------------------
#endif //PLUGIN_RUNNER_H
