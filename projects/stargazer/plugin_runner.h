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
#include <pthread.h>
#include <string>

#include "base_plugin.h"
#include "base_settings.h"
#include "traffcounter.h"
#include "tariffs.h"
#include "admins.h"
#include "users.h"

using namespace std;
//-----------------------------------------------------------------------------
class PLUGIN_RUNNER
{
public:
    PLUGIN_RUNNER(const string & pluginFileName,
                  const MODULE_SETTINGS & ms,
                  ADMINS * admins,
                  TARIFFS * tariffs,
                  USERS * users,
                  TRAFFCOUNTER * tc,
                  BASE_STORE * store,
                  const SETTINGS * s);
    PLUGIN_RUNNER(const PLUGIN_RUNNER & rvalue);
    ~PLUGIN_RUNNER();

    PLUGIN_RUNNER & operator=(const PLUGIN_RUNNER & rvalue);

    int             Start();
    int             Stop();
    int             Reload();
    int             Restart();
    bool            IsRunning();

    const string &  GetStrError() const;
    BASE_PLUGIN *   GetPlugin();
    const string & GetFileName() const { return pluginFileName; };

    int             Load();
    int             Unload();

    uint16_t        GetStartPosition() const;
    uint16_t        GetStopPosition() const;

private:
    string          pluginFileName;
    string          pluginSettingFileName;

    BASE_PLUGIN *   plugin;
    int             isPluginLoaded;
    string          errorStr;

    void *          libHandle;
    bool            isRunning;

    ADMINS *        admins;
    TARIFFS *       tariffs;
    USERS *         users;
    BASE_STORE *    store;
    TRAFFCOUNTER *  traffCnt;
    const SETTINGS * stgSettings;
    MODULE_SETTINGS modSettings;
};
//-----------------------------------------------------------------------------
#endif //PLUGIN_RUNNER_H


