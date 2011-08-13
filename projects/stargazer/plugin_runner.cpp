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

#include "stg/common.h"
#include "stg/traffcounter.h"
#include "plugin_runner.h"
#include "settings_impl.h"
#include "admins_impl.h"
#include "tariffs_impl.h"
#include "users_impl.h"
#include "services_impl.h"
#include "corps_impl.h"

//-----------------------------------------------------------------------------
PLUGIN_RUNNER::PLUGIN_RUNNER(const std::string & pFileName,
                             const MODULE_SETTINGS & ms,
                             ADMINS_IMPL * a,
                             TARIFFS_IMPL * t,
                             USERS_IMPL * u,
                             SERVICES_IMPL * svc,
                             CORPORATIONS_IMPL * crp,
                             TRAFFCOUNTER * tc,
                             STORE * st,
                             const SETTINGS_IMPL * s)
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
      services(svc),
      corps(crp),
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
      services(rvalue.services),
      corps(rvalue.corps),
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
services = rvalue.services;
corps = rvalue.corps;
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

isPluginLoaded = false;
}
//-----------------------------------------------------------------------------
PLUGIN * PLUGIN_RUNNER::GetPlugin()
{
if (!isPluginLoaded)
    {
    errorStr = "Plugin '" + pluginFileName + "' is not loaded yet!";
    printfd(__FILE__, "PLUGIN_LOADER::GetPlugin() - %s\n", errorStr.c_str());
    return NULL;
    }

return plugin;
}
//-----------------------------------------------------------------------------
int PLUGIN_RUNNER::Start()
{
if (!isPluginLoaded)
    if (Load())
        return -1;

if (!plugin)
    {
    errorStr = "Plugin '" + pluginFileName + "' was not created!";
    printfd(__FILE__, "PLUGIN_LOADER::Start() - %s\n", errorStr.c_str());
    return -1;
    }

plugin->SetTariffs(tariffs);
plugin->SetAdmins(admins);
plugin->SetUsers(users);
plugin->SetServices(services);
plugin->SetCorporations(corps);
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
if (!isPluginLoaded)
    {
    errorStr = "Plugin '" + pluginFileName + "' was not loaded yet!";
    printfd(__FILE__, "PLUGIN_LOADER::Stop() - %s\n", errorStr.c_str());
    return -1;
    }

if (!plugin)
    {
    errorStr = "Plugin '" + pluginFileName + "' was not created!";
    printfd(__FILE__, "PLUGIN_LOADER::Stop() - %s\n", errorStr.c_str());
    return -1;
    }

plugin->Stop();

return 0;
}
//-----------------------------------------------------------------------------
int PLUGIN_RUNNER::Reload()
{
if (!isPluginLoaded)
    {
    errorStr = "Plugin '" + pluginFileName + "' was not loaded yet!";
    printfd(__FILE__, "PLUGIN_LOADER::Reload() - %s\n", errorStr.c_str());
    return -1;
    }

if (!plugin)
    {
    errorStr = "Plugin '" + pluginFileName + "' was not created!";
    printfd(__FILE__, "PLUGIN_LOADER::Reload() - %s\n", errorStr.c_str());
    return -1;
    }

int res = plugin->Reload();
errorStr = plugin->GetStrError();
return res;
}
//-----------------------------------------------------------------------------
bool PLUGIN_RUNNER::IsRunning()
{
if (!isPluginLoaded)
    {
    errorStr = "Plugin '" + pluginFileName + "' was not loaded yet!";
    printfd(__FILE__, "PLUGIN_LOADER::IsRunning() - %s\n", errorStr.c_str());
    return false;
    }

if (!plugin)
    {
    errorStr = "Plugin '" + pluginFileName + "' was not created!";
    printfd(__FILE__, "PLUGIN_LOADER::IsRunning() - %s\n", errorStr.c_str());
    return false;
    }

return plugin->IsRunning();
}
//-----------------------------------------------------------------------------
int PLUGIN_RUNNER::Load()
{
if (isPluginLoaded)
    {
    errorStr = "Plugin '" + pluginFileName + "' was already loaded!";
    printfd(__FILE__, "PLUGIN_LOADER::Load() - %s\n", errorStr.c_str());
    return -1;
    }

if (pluginFileName.empty())
    {
    errorStr = "Empty plugin file name!";
    printfd(__FILE__, "PLUGIN_LOADER::Load() - %s\n", errorStr.c_str());
    return -1;
    }

libHandle = dlopen(pluginFileName.c_str(), RTLD_NOW);

if (!libHandle)
    {
    errorStr = "Error loading plugin '"
        + pluginFileName + "': '" + dlerror() + "'";
    printfd(__FILE__, "PLUGIN_LOADER::Load() - %s\n", errorStr.c_str());
    return -1;
    }

isPluginLoaded = true;

PLUGIN * (*GetPlugin)();
GetPlugin = (PLUGIN * (*)())dlsym(libHandle, "GetPlugin");
if (!GetPlugin)
    {
    errorStr = std::string("GetPlugin() not found. ") + dlerror();
    printfd(__FILE__, "PLUGIN_LOADER::Load() - %s\n", errorStr.c_str());
    return -1;
    }
plugin = GetPlugin();

if (!plugin)
    {
    errorStr = "Plugin was not created!";
    printfd(__FILE__, "PLUGIN_LOADER::Load() - %s\n", errorStr.c_str());
    return -1;
    }

plugin->SetSettings(modSettings);
if (plugin->ParseSettings())
    {
    errorStr = plugin->GetStrError();
    printfd(__FILE__, "PLUGIN_LOADER::Load() - Failed to parse settings. Plugin reports: '%s'\n", errorStr.c_str());
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
        errorStr = "Failed to unload plugin '";
        errorStr += pluginFileName + "': ";
        errorStr += dlerror();
        printfd(__FILE__, "PLUGIN_LOADER::Unload() - %s", errorStr.c_str());
        return -1;
        }
    plugin = NULL;
    isPluginLoaded = false;
    }
return 0;
}
//-----------------------------------------------------------------------------
