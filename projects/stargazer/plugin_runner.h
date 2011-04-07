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
 $Revision: 1.13 $
 $Date: 2010/03/04 12:22:41 $
 $Author: faust $
 */

#ifndef PLUGIN_RUNNER_H
#define PLUGIN_RUNNER_H

#include <string>

#include "stg/module_settings.h"
#include "stg/plugin.h"
#include "stg/os_int.h"

class SETTINGS_IMPL;
class ADMINS_IMPL;
class TARIFFS_IMPL;
class USERS_IMPL;
class TRAFFCOUNTER;
class STORE;

//-----------------------------------------------------------------------------
class PLUGIN_RUNNER {
public:
    PLUGIN_RUNNER(const std::string & pluginFileName,
                  const MODULE_SETTINGS & ms,
                  ADMINS_IMPL * admins,
                  TARIFFS_IMPL * tariffs,
                  USERS_IMPL * users,
                  TRAFFCOUNTER * tc,
                  STORE * store,
                  const SETTINGS_IMPL * s);
    PLUGIN_RUNNER(const PLUGIN_RUNNER & rvalue);
    ~PLUGIN_RUNNER();

    PLUGIN_RUNNER & operator=(const PLUGIN_RUNNER & rvalue);

    int             Start();
    int             Stop();
    int             Reload();
    int             Restart();
    bool            IsRunning();

    const std::string & GetStrError() const { return errorStr; }
    PLUGIN *        GetPlugin();
    const std::string & GetFileName() const { return pluginFileName; }

    int             Load();
    int             Unload();

    uint16_t        GetStartPosition() const { return plugin->GetStartPosition(); }
    uint16_t        GetStopPosition() const { return plugin->GetStopPosition(); }

private:
    std::string     pluginFileName;
    std::string     pluginSettingFileName;

    PLUGIN *        plugin;
    bool            isPluginLoaded;
    std::string     errorStr;

    void *          libHandle;
    bool            isRunning;

    ADMINS_IMPL *   admins;
    TARIFFS_IMPL *  tariffs;
    USERS_IMPL *    users;
    STORE *         store;
    TRAFFCOUNTER *  traffCnt;
    const SETTINGS_IMPL * stgSettings;
    MODULE_SETTINGS modSettings;
};
//-----------------------------------------------------------------------------
#endif //PLUGIN_RUNNER_H
