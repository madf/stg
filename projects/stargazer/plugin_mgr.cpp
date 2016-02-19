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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#include "plugin_mgr.h"

#include "plugin_runner.h"

#include "admins_impl.h"
#include "tariffs_impl.h"
#include "services_impl.h"
#include "corps_impl.h"
#include "users_impl.h"
#include "traffcounter_impl.h"
#include "settings_impl.h"

#include "stg/logger.h"

using STG::PluginManager;

namespace
{

bool StartModCmp(const PLUGIN_RUNNER * lhs, const PLUGIN_RUNNER * rhs)
{
    return lhs->GetStartPosition() < rhs->GetStartPosition();
}

bool StopModCmp(const PLUGIN_RUNNER * lhs, const PLUGIN_RUNNER * rhs)
{
    return lhs->GetStopPosition() > rhs->GetStopPosition();
}

} // namespace anonymous

PluginManager::PluginManager(const SETTINGS_IMPL& settings,
                             STORE& store, ADMINS_IMPL& admins, TARIFFS_IMPL& tariffs,
                             SERVICES_IMPL& services, CORPORATIONS_IMPL& corporations,
                             USERS_IMPL& users, TRAFFCOUNTER_IMPL& traffcounter)
    : m_log(GetStgLogger())
{
    std::string basePath = settings.GetModulesPath();
    const std::vector<MODULE_SETTINGS> & modSettings(settings.GetModulesSettings());
    for (size_t i = 0; i < modSettings.size(); i++)
    {
        std::string modulePath = basePath + "/mod_" + modSettings[i].moduleName + ".so";
        std::string moduleName = modSettings[i].moduleName;
        printfd(__FILE__, "Module: %s\n", modulePath.c_str());
        try
        {
            m_modules.push_back(
                new PLUGIN_RUNNER(modulePath, moduleName, modSettings[i], admins, tariffs,
                                  users, services, corporations, traffcounter,
                                  store, settings)
            );
        }
        catch (const PLUGIN_RUNNER::Error & ex)
        {
            m_log(ex.what());
            printfd(__FILE__, "%s\n", ex.what());
            // TODO: React
        }
    }
    std::sort(m_modules.begin(), m_modules.end(), StartModCmp);
    for (size_t i = 0; i < m_modules.size(); ++i)
    {
        PLUGIN & plugin = m_modules[i]->GetPlugin();
        if (m_modules[i]->Start())
        {
            m_log("Failed to start module '%s': '%s'", plugin.GetVersion().c_str(),
                                                       plugin.GetStrError().c_str());
            printfd(__FILE__, "Failed to start module '%s': '%s'\n", plugin.GetVersion().c_str(),
                                                                   plugin.GetStrError().c_str());
        }
        else
        {
            m_log("Module '%s' started successfully.", plugin.GetVersion().c_str());
            printfd(__FILE__, "Module '%s' started successfully.\n", plugin.GetVersion().c_str());
        }
    }
}

PluginManager::~PluginManager()
{
    std::sort(m_modules.begin(), m_modules.end(), StopModCmp);
    for (size_t i = 0; i < m_modules.size(); ++i)
    {
        PLUGIN & plugin = m_modules[i]->GetPlugin();
        if (m_modules[i]->Stop())
        {
            m_log("Failed to stop module '%s': '%s'", plugin.GetVersion().c_str(),
                                                      plugin.GetStrError().c_str());
            printfd(__FILE__, "Failed to stop module '%s': '%s'\n", plugin.GetVersion().c_str(),
                                                                  plugin.GetStrError().c_str());
        }
        else
        {
            m_log("Module '%s' stopped successfully.", plugin.GetVersion().c_str());
            printfd(__FILE__, "Module '%s' stopped successfully.\n", plugin.GetVersion().c_str());
        }
    }
    for (size_t i = 0; i < m_modules.size(); ++i)
        delete m_modules[i];
}

void PluginManager::reload(const SETTINGS_IMPL& settings)
{
    const std::vector<MODULE_SETTINGS> & modSettings(settings.GetModulesSettings());
    for (size_t i = 0; i < m_modules.size(); ++i)
    {
        for (size_t j = 0; j < modSettings.size(); j++)
        {
           if (modSettings[j].moduleName == m_modules[i]->GetName()) 
           {
                PLUGIN & plugin = m_modules[i]->GetPlugin();
                if (m_modules[i]->Reload(modSettings[j]))
                {
                    m_log("Error reloading module '%s': '%s'", plugin.GetVersion().c_str(),
                                                               plugin.GetStrError().c_str());
                    printfd(__FILE__, "Error reloading module '%s': '%s'\n", plugin.GetVersion().c_str(),
                                                                             plugin.GetStrError().c_str());
                }
           }
        }
    }
}
