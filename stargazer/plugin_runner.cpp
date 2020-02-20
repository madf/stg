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

using STG::PluginRunner;
using STG::Plugin;

//-----------------------------------------------------------------------------
PluginRunner::PluginRunner(const std::string& fileName,
                           const std::string& name,
                           const ModuleSettings& ms,
                           Admins& admins,
                           Tariffs& tariffs,
                           Users& users,
                           Services& services,
                           Corporations& corporations,
                           TraffCounter& traffcounter,
                           Store& store,
                           const Settings& settings)
    : pluginFileName(fileName),
      pluginName(name),
      libHandle(NULL),
      m_plugin(load(ms, admins, tariffs, users, services, corporations,
                    traffcounter, store, settings))
{
}
//-----------------------------------------------------------------------------
PluginRunner::~PluginRunner()
{
    delete &m_plugin;
    if (dlclose(libHandle))
    {
        errorStr = "Failed to unload plugin '" + pluginFileName + "': " + dlerror();
        printfd(__FILE__, "PluginRunner::Unload() - %s", errorStr.c_str());
    }
}
//-----------------------------------------------------------------------------
int PluginRunner::Start()
{
    int res = m_plugin.Start();
    errorStr = m_plugin.GetStrError();
    return res;
}
//-----------------------------------------------------------------------------
int PluginRunner::Stop()
{
    int res = m_plugin.Stop();
    errorStr = m_plugin.GetStrError();
    return res;
}
//-----------------------------------------------------------------------------
int PluginRunner::Reload(const ModuleSettings& ms)
{
    int res = m_plugin.Reload(ms);
    errorStr = m_plugin.GetStrError();
    return res;
}
//-----------------------------------------------------------------------------
Plugin & PluginRunner::load(const ModuleSettings& ms,
                            Admins& admins,
                            Tariffs& tariffs,
                            Users& users,
                            Services& services,
                            Corporations& corporations,
                            TraffCounter& traffcounter,
                            Store& store,
                            const Settings& settings)
{
    if (pluginFileName.empty())
    {
        const std::string msg = "Empty plugin file name.";
        printfd(__FILE__, "PluginRunner::load() - %s\n", msg.c_str());
        throw Error(msg);
    }

    if (access(pluginFileName.c_str(), R_OK))
    {
        const std::string msg = "Plugin file '" + pluginFileName + "' is missing or inaccessible.";
        printfd(__FILE__, "PluginRunner::load() - %s\n", msg.c_str());
        throw Error(msg);
    }

    libHandle = dlopen(pluginFileName.c_str(), RTLD_NOW);

    if (!libHandle)
    {
        std::string msg = "Error loading plugin '" + pluginFileName + "'";
        const char* error = dlerror();
        if (error)
            msg = msg + ": '" + error + "'";
        printfd(__FILE__, "PluginRunner::load() - %s\n", msg.c_str());
        throw Error(msg);
    }

    using Getter = Plugin* (*)();
    auto GetPlugin = reinterpret_cast<Getter>(dlsym(libHandle, "GetPlugin"));
    if (!GetPlugin)
    {
        const std::string msg = "Plugin '" + pluginFileName + "' does not have GetPlugin() function. ";
        printfd(__FILE__, "PluginRunner::load() - %s\n", msg.c_str());
        throw Error(msg);
    }

    Plugin* plugin = GetPlugin();

    if (!plugin)
    {
        const std::string msg = "Failed to create an instance of plugin '" + pluginFileName + "'.";
        printfd(__FILE__, "PluginRunner::load() - %s\n", msg.c_str());
        throw Error(msg);
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
        const std::string msg = "Plugin '" + pluginFileName + "' is unable to parse settings. " + plugin->GetStrError();
        printfd(__FILE__, "PluginRunner::load() - %s\n", msg.c_str());
        throw Error(msg);
    }

    return *plugin;
}
