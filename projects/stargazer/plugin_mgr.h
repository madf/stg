#ifndef __STG_PLUGIN_MGR_H__
#define __STG_PLUGIN_MGR_H__

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

#include "stg/module_settings.h"

#include <vector>

class SETTINGS_IMPL;
class PLUGIN_RUNNER;
class STORE;
class ADMINS_IMPL;
class TARIFFS_IMPL;
class SERVICES_IMPL;
class CORPORATIONS_IMPL;
class USERS_IMPL;
class TRAFFCOUNTER_IMPL;
class STG_LOGGER;

namespace STG
{

class PluginManager
{
    public:
        PluginManager(const SETTINGS_IMPL& settings,
                      STORE& store, ADMINS_IMPL& admins, TARIFFS_IMPL& tariffs,
                      SERVICES_IMPL& services, CORPORATIONS_IMPL& corporations,
                      USERS_IMPL& users, TRAFFCOUNTER_IMPL& traffcounter);
        ~PluginManager();

        void reload(const SETTINGS_IMPL& settings);

    private:
        std::vector<PLUGIN_RUNNER*> m_modules;
        STG_LOGGER & m_log;
};

} // namespace STG

#endif
