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
using STG::PluginRunner;

namespace
{

bool StartModCmp(const PluginRunner * lhs, const PluginRunner * rhs)
{
    return lhs->GetStartPosition() < rhs->GetStartPosition();
}

bool StopModCmp(const PluginRunner * lhs, const PluginRunner * rhs)
{
    return lhs->GetStopPosition() > rhs->GetStopPosition();
}

} // namespace anonymous

PluginManager::PluginManager(const SettingsImpl& settings,
                             Store& store, AdminsImpl& admins, TariffsImpl& tariffs,
                             ServicesImpl& services, CorporationsImpl& corporations,
                             UsersImpl& users, TraffCounterImpl& traffcounter)
    : m_log(Logger::get())
{
    std::string basePath = settings.GetModulesPath();
    const std::vector<ModuleSettings> & modSettings(settings.GetModulesSettings());
    for (size_t i = 0; i < modSettings.size(); i++)
    {
        std::string moduleName = modSettings[i].moduleName;
        std::string modulePath = basePath + "/mod_" + moduleName + ".so";
        printfd(__FILE__, "Module: %s\n", modulePath.c_str());
        try
        {
            m_modules.push_back(
                new PluginRunner(modulePath, moduleName, modSettings[i], admins, tariffs,
                                  users, services, corporations, traffcounter,
                                  store, settings)
            );
        }
        catch (const PluginRunner::Error & ex)
        {
            m_log(ex.what());
            printfd(__FILE__, "%s\n", ex.what());
            // TODO: React
        }
    }
    std::sort(m_modules.begin(), m_modules.end(), StartModCmp);
    for (size_t i = 0; i < m_modules.size(); ++i)
    {
        auto& plugin = m_modules[i]->GetPlugin();
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
    stop();
    for (size_t i = 0; i < m_modules.size(); ++i)
        delete m_modules[i];
}

void PluginManager::reload(const SettingsImpl& settings)
{
    const std::vector<ModuleSettings> & modSettings(settings.GetModulesSettings());
    for (size_t i = 0; i < m_modules.size(); ++i)
    {
        for (size_t j = 0; j < modSettings.size(); j++)
        {
            if (modSettings[j].moduleName == m_modules[i]->GetName())
            {
                auto& plugin = m_modules[i]->GetPlugin();
                if (m_modules[i]->Reload(modSettings[j]))
                {
                    m_log("Error reloading module '%s': '%s'", plugin.GetVersion().c_str(),
                                                               plugin.GetStrError().c_str());
                    printfd(__FILE__, "Error reloading module '%s': '%s'\n", plugin.GetVersion().c_str(),
                                                                             plugin.GetStrError().c_str());
                }
                break;
            }
        }
    }
}

void PluginManager::stop()
{
    std::sort(m_modules.begin(), m_modules.end(), StopModCmp);
    for (size_t i = 0; i < m_modules.size(); ++i)
    {
        if (!m_modules[i]->IsRunning())
            continue;
        auto& plugin = m_modules[i]->GetPlugin();
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
}
