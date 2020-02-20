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

#include <dlfcn.h>

#include "stg/common.h"
#include "stg/store.h"
#include "store_loader.h"
#include "settings_impl.h"

using STG::StoreLoader;

StoreLoader::StoreLoader(const SettingsImpl& settings) noexcept
    : isLoaded(false),
      handle(NULL),
      plugin(NULL),
      storeSettings(settings.GetStoreModuleSettings()),
      pluginFileName(settings.GetModulesPath() + "/mod_" + storeSettings.moduleName + ".so")
{
}

StoreLoader::~StoreLoader()
{
    unload();
}

bool StoreLoader::load() noexcept
{
    if (isLoaded)
    {
        errorStr = "Store plugin '" + pluginFileName + "' was already loaded!";
        printfd(__FILE__, "StoreLoader::load() - %s\n", errorStr.c_str());
        return false;
    }

    if (pluginFileName.empty())
    {
        errorStr = "Empty store plugin filename";
        printfd(__FILE__, "StoreLoader::load() - %s\n", errorStr.c_str());
        return true;
    }

    handle = dlopen(pluginFileName.c_str(), RTLD_NOW);

    if (!handle)
    {
        errorStr = "Error loading plugin '"
            + pluginFileName + "': '" + dlerror() + "'";
        printfd(__FILE__, "StoreLoader::Load() - %s\n", errorStr.c_str());
        return true;
    }

    isLoaded = true;

    using Getter = Store* (*)();
    auto GetStore = reinterpret_cast<Getter>(dlsym(handle, "GetStore"));
    if (!GetStore)
    {
        errorStr = std::string("GetStore() not found! ") + dlerror();
        printfd(__FILE__, "StoreLoader::load() - %s\n", errorStr.c_str());
        return true;
    }

    plugin = GetStore();

    if (!plugin)
    {
        errorStr = "Plugin was not created!";
        printfd(__FILE__, "StoreLoader::Load() - %s\n");
        return true;
    }

    plugin->SetSettings(storeSettings);
    if (plugin->ParseSettings())
    {
        errorStr = plugin->GetStrError();
        printfd(__FILE__, "StoreLoader::Load() - Failed to parse settings. Plugin reports: '%s'\n", errorStr.c_str());
        return true;
    }

    return false;
}

bool StoreLoader::unload() noexcept
{
    if (!isLoaded)
        return true;

    delete plugin;

    if (dlclose(handle))
    {
        errorStr = "Failed to unload plugin '";
        errorStr += pluginFileName + "': ";
        errorStr += dlerror();
        printfd(__FILE__, "StoreLoader::Unload() - %s\n", errorStr.c_str());
        return true;
    }

    isLoaded = false;

    return false;
}
