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

#include "plugin_runner.h"

#include "stg/common.h"

#include <dlfcn.h>
#include <unistd.h>

//-----------------------------------------------------------------------------
PLUGIN_RUNNER::PLUGIN_RUNNER(const std::string & fileName,
                             const MODULE_SETTINGS & ms,
                             ADMINS & admins,
                             TARIFFS & tariffs,
                             USERS & users,
                             SERVICES & services,
                             CORPORATIONS & corporations,
                             TRAFFCOUNTER & traffcounter,
                             STORE & store,
                             const SETTINGS & settings)
    : pluginFileName(fileName),
      libHandle(NULL),
      m_plugin(Load(ms, admins, tariffs, users, services, corporations,
                    traffcounter, store, settings))
{
}
//-----------------------------------------------------------------------------
PLUGIN_RUNNER::~PLUGIN_RUNNER()
{
if (dlclose(libHandle))
    {
    errorStr = "Failed to unload plugin '" + pluginFileName + "': " + dlerror();
    printfd(__FILE__, "PLUGIN_RUNNER::Unload() - %s", errorStr.c_str());
    }
}
//-----------------------------------------------------------------------------
int PLUGIN_RUNNER::Start()
{
int res = m_plugin.Start();
errorStr = m_plugin.GetStrError();
return res;
}
//-----------------------------------------------------------------------------
int PLUGIN_RUNNER::Stop()
{
int res = m_plugin.Stop();
errorStr = m_plugin.GetStrError();
return res;
}
//-----------------------------------------------------------------------------
int PLUGIN_RUNNER::Reload()
{
int res = m_plugin.Reload();
errorStr = m_plugin.GetStrError();
return res;
}
//-----------------------------------------------------------------------------
PLUGIN & PLUGIN_RUNNER::Load(const MODULE_SETTINGS & ms,
                             ADMINS & admins,
                             TARIFFS & tariffs,
                             USERS & users,
                             SERVICES & services,
                             CORPORATIONS & corporations,
                             TRAFFCOUNTER & traffcounter,
                             STORE & store,
                             const SETTINGS & settings)
{
if (pluginFileName.empty())
    {
    errorStr = "Empty plugin file name.";
    printfd(__FILE__, "PLUGIN_RUNNER::Load() - %s\n", errorStr.c_str());
    throw Error(errorStr);
    }

libHandle = dlopen(pluginFileName.c_str(), RTLD_NOW);

if (!libHandle)
    {
    errorStr = "Error loading plugin '" + pluginFileName + "': '" + dlerror() + "'";
    printfd(__FILE__, "PLUGIN_RUNNER::Load() - %s\n", errorStr.c_str());
    throw Error(errorStr);
    }

PLUGIN * (*GetPlugin)();
GetPlugin = (PLUGIN * (*)())dlsym(libHandle, "GetPlugin");
if (!GetPlugin)
    {
    errorStr = "Plugin '" + pluginFileName + "' does not have GetPlugin() function. " + dlerror();
    printfd(__FILE__, "PLUGIN_RUNNER::Load() - %s\n", errorStr.c_str());
    throw Error(errorStr);
    }
PLUGIN * plugin = GetPlugin();

if (!plugin)
    {
    errorStr = "Failed to create an instance of plugin '" + pluginFileName + "'.";
    printfd(__FILE__, "PLUGIN_RUNNER::Load() - %s\n", errorStr.c_str());
    throw Error(errorStr);
    }

plugin->SetSettings(ms);
plugin->SetTariffs(&tariffs);
plugin->SetAdmins(&admins);
plugin->SetUsers(&users);
plugin->SetServices(&services);
plugin->SetCorporations(&corporations);
plugin->SetTraffcounter(&traffcounter);
plugin->SetStore(&store);
plugin->SetStgSettings(&settings);

if (plugin->ParseSettings())
    {
    errorStr = "Plugin '" + pluginFileName + "' is unable to parse settings. " + plugin->GetStrError();
    printfd(__FILE__, "PLUGIN_RUNNER::Load() - %s\n", errorStr.c_str());
    throw Error(errorStr);
    }

return *plugin;
}
