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

#include <vector>

namespace STG
{

class SettingsImpl;
class PluginRunner;
struct Store;
class AdminsImpl;
class TariffsImpl;
class ServicesImpl;
class CorporationsImpl;
class UsersImpl;
class TraffCounterImpl;
class Logger;

class PluginManager
{
    public:
        PluginManager(const SettingsImpl& settings,
                      Store& store, AdminsImpl& admins, TariffsImpl& tariffs,
                      ServicesImpl& services, CorporationsImpl& corporations,
                      UsersImpl& users, TraffCounterImpl& traffcounter);
        ~PluginManager();

        void reload(const SettingsImpl& settings);
        void stop();

    private:
        std::vector<PluginRunner*> m_modules;
        Logger & m_log;
};

}
