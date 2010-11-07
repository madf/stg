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
 $Revision: 1.17 $
 $Date: 2010/09/13 05:52:46 $
 $Author: faust $
 */

#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>

#include "plugin_runner.h"
#include "common.h"
#include "conffiles.h"

//-----------------------------------------------------------------------------
PLUGIN_RUNNER::PLUGIN_RUNNER(const string & pFileName,
                             const MODULE_SETTINGS & ms,
                             ADMINS * a,
                             TARIFFS * t,
                             USERS * u,
                             TRAFFCOUNTER * tc,
                             BASE_STORE * st,
                             const SETTINGS * s)
    : pluginFileName(pFileName),
      pluginSettingFileName(),
      plugin(NULL),
      isPluginLoaded(false),
      errorStr(),
      libHandle(NULL),
      isRunning(false),
      admins(a),
      tariffs(t),
      users(u),
      store(st),
      traffCnt(tc),
      stgSettings(s),
      modSettings(ms)
{
}
//-----------------------------------------------------------------------------
PLUGIN_RUNNER::PLUGIN_RUNNER(const PLUGIN_RUNNER & rvalue)
    : pluginFileName(rvalue.pluginFileName),
      pluginSettingFileName(rvalue.pluginSettingFileName),
      plugin(rvalue.plugin),
      isPluginLoaded(rvalue.isPluginLoaded),
      errorStr(rvalue.errorStr),
      libHandle(rvalue.libHandle),
      isRunning(rvalue.isRunning),
      admins(rvalue.admins),
      tariffs(rvalue.tariffs),
      users(rvalue.users),
      store(rvalue.store),
      traffCnt(rvalue.traffCnt),
      stgSettings(rvalue.stgSettings),
      modSettings(rvalue.modSettings)
{
}
//-----------------------------------------------------------------------------
PLUGIN_RUNNER & PLUGIN_RUNNER::operator=(const PLUGIN_RUNNER & rvalue)
{
pluginFileName = rvalue.pluginFileName;
pluginSettingFileName = rvalue.pluginSettingFileName;
plugin = rvalue.plugin;
isPluginLoaded = rvalue.isPluginLoaded;
errorStr = rvalue.errorStr;
libHandle = rvalue.libHandle;
isRunning = rvalue.isRunning;
admins = rvalue.admins;
tariffs = rvalue.tariffs;
users = rvalue.users;
store = rvalue.store;
traffCnt = rvalue.traffCnt;
stgSettings = rvalue.stgSettings;
modSettings = rvalue.modSettings;

return *this;
}
//-----------------------------------------------------------------------------
PLUGIN_RUNNER::~PLUGIN_RUNNER()
{
if (isPluginLoaded)
    {
    Unload();
    }

isPluginLoaded = 0;
}
//-----------------------------------------------------------------------------
BASE_PLUGIN * PLUGIN_RUNNER::GetPlugin()
{
return plugin;
}
//-----------------------------------------------------------------------------
int PLUGIN_RUNNER::Start()
{
if (!isPluginLoaded)
    if (Load())
        return -1;

plugin->SetTariffs(tariffs);
plugin->SetAdmins(admins);
plugin->SetUsers(users);
plugin->SetTraffcounter(traffCnt);
plugin->SetStore(store);
plugin->SetStgSettings(stgSettings);

if (plugin->Start())
    {
    errorStr = plugin->GetStrError();
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int PLUGIN_RUNNER::Stop()
{
plugin->Stop();

//if (Unload())
//    return -1;
return 0;
}
//-----------------------------------------------------------------------------
int PLUGIN_RUNNER::Reload()
{
int res = plugin->Reload();
errorStr = plugin->GetStrError();
return res;
}
//-----------------------------------------------------------------------------
bool PLUGIN_RUNNER::IsRunning()
{
if (!isPluginLoaded)
    return false;
return plugin->IsRunning();
}
//-----------------------------------------------------------------------------
int PLUGIN_RUNNER::Load()
{
if (!pluginFileName.size())
    {
    errorStr = "Plugin loading failed. No plugin";
    printfd(__FILE__, "%s\n", errorStr.c_str());
    return -1;
    }

libHandle = dlopen(pluginFileName.c_str(), RTLD_NOW);

if (!libHandle)
    {
    errorStr = string("Plugin loading failed. ") + dlerror();
    printfd(__FILE__, "%s\n", errorStr.c_str());
    return -1;
    }

BASE_PLUGIN * (*GetPlugin)();
GetPlugin = (BASE_PLUGIN * (*)())dlsym(libHandle, "GetPlugin");
if (!GetPlugin)
    {
    errorStr = string("GetPlugin() not found. ") + dlerror();
    return -1;
    }
plugin = GetPlugin();
isPluginLoaded++;

if (!plugin)
    {
    errorStr = "Plugin was not created!";
    printfd(__FILE__, "%s\n", errorStr.c_str());
    return -1;
    }

plugin->SetSettings(modSettings);
printfd(__FILE__, "Plugin %s parsesettings\n", plugin->GetVersion().c_str());
if (plugin->ParseSettings())
    {
    errorStr = "Plugin \'" + plugin->GetVersion() + "\' error: " + plugin->GetStrError();
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int PLUGIN_RUNNER::Unload()
{
if (isPluginLoaded)
    {
    if (dlclose(libHandle))
        {
        errorStr = dlerror();
        printfd(__FILE__, "Error unloading plugin '%s': '%s'", pluginFileName.c_str(), dlerror());
        return -1;
        }
    isPluginLoaded--;
    }
return 0;
}
//-----------------------------------------------------------------------------
const string & PLUGIN_RUNNER::GetStrError() const
{
return errorStr;
}
//-----------------------------------------------------------------------------
uint16_t PLUGIN_RUNNER::GetStartPosition() const
{
return plugin->GetStartPosition();
}
//-----------------------------------------------------------------------------
uint16_t PLUGIN_RUNNER::GetStopPosition() const
{
return plugin->GetStopPosition();
}
//-----------------------------------------------------------------------------

