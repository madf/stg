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

/*
 $Revision: 1.6 $
 $Date: 2010/03/04 12:24:19 $
 $Author: faust $
 */

/*
 *  An implementation of RAII store plugin loader
 */

#include <dlfcn.h>

#include "store_loader.h"
#include "common.h"
#include "store.h"
#include "settings.h"

STORE_LOADER::STORE_LOADER(const SETTINGS & settings)
    : isLoaded(false),
      handle(NULL),
      plugin(NULL),
      errorStr(),
      storeSettings(settings.GetStoreModuleSettings()),
      pluginFileName(settings.GetModulesPath() + "/mod_" + storeSettings.moduleName + ".so")
{
}

STORE_LOADER::~STORE_LOADER()
{
Unload();
}

bool STORE_LOADER::Load()
{
printfd(__FILE__, "STORE_LOADER::Load()\n");
if (isLoaded)
    {
    return false;
    }

if (pluginFileName.empty())
    {
    errorStr = "Empty store plugin filename";
    printfd(__FILE__, "STORE_LOADER::Load - %s\n", errorStr.c_str());
    return true;
    }

handle = dlopen(pluginFileName.c_str(), RTLD_NOW);

if (!handle)
    {
    errorStr = "Error loading plugin '"
        + pluginFileName + "': '" + dlerror() + "'";
    printfd(__FILE__, "STORE_LOADER::Load - %s\n", errorStr.c_str());
    return true;
    }

isLoaded = true;

STORE * (*GetStore)();
GetStore = (STORE * (*)())dlsym(handle, "GetStore");
if (!GetStore)
    {
    errorStr = "GetStore not found.";
    printfd(__FILE__, "STORE_LOADER::Load - %s\n", errorStr.c_str());
    return true;
    }

plugin = GetStore();

if (!plugin)
    {
    errorStr = "NULL store plugin";
    printfd(__FILE__, "STORE_LOADER::Load - %s\n");
    return true;
    }

plugin->SetSettings(storeSettings);
if (plugin->ParseSettings())
    {
    errorStr = plugin->GetStrError();
    printfd(__FILE__, "Failed to parse settings. Plugin reports: '%s'\n", errorStr.c_str());
    return true;
    }

return false;
}

bool STORE_LOADER::Unload()
{
printfd(__FILE__, "STORE_LOADER::Unload()\n");
if (!isLoaded)
    {
    return false;
    }

if (dlclose(handle))
    {
    errorStr = "Failed to unload plugin: '";
    errorStr += dlerror();
    errorStr += "'";
    printfd(__FILE__, "STORE_LOADER::Unload - %s\n", errorStr.c_str());
    return true;
    }

isLoaded = false;

return false;
}
