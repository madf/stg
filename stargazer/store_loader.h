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

#pragma once

#include "stg/module_settings.h"

#include <string>

namespace STG
{

struct Store;
class SettingsImpl;

class StoreLoader {
    public:
        explicit StoreLoader(const SettingsImpl& settings) noexcept;
        ~StoreLoader();

        StoreLoader(const StoreLoader&) = delete;
        StoreLoader& operator=(const StoreLoader&) = delete;

        bool load() noexcept;
        bool unload() noexcept;

        Store& get() noexcept { return *plugin; }

        const std::string& GetStrError() const noexcept { return errorStr; }

    private:
        bool isLoaded;
        void* handle;
        Store* plugin;
        std::string errorStr;
        ModuleSettings storeSettings;
        std::string pluginFileName;
};

}
