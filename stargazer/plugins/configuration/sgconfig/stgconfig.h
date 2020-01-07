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

#ifndef STGCONFIG_H
#define STGCONFIG_H

#include "configproto.h"

#include "stg/plugin.h"
#include "stg/logger.h"

#include <string>

#include <pthread.h>

class STG_CONFIG_SETTINGS
{
    public:
        STG_CONFIG_SETTINGS() : m_port(0), m_bindAddress("0.0.0.0") {}
        virtual ~STG_CONFIG_SETTINGS() {}
        const std::string & GetStrError() const { return errorStr; }
        bool ParseSettings(const MODULE_SETTINGS & s);
        uint16_t GetPort() const { return m_port; }
        const std::string & GetBindAddress() const { return m_bindAddress; }
    private:
        std::string errorStr;
        uint16_t m_port;
        std::string m_bindAddress;
};

class STG_CONFIG : public PLUGIN
{
    public:
        STG_CONFIG();

        void                SetUsers(USERS * users) { config.SetUsers(users); }
        void                SetTariffs(TARIFFS * tariffs) { config.SetTariffs(tariffs); }
        void                SetAdmins(ADMINS * admins) { config.SetAdmins(admins); }
        void                SetServices(SERVICES * services) { config.SetServices(services); }
        void                SetCorporations(CORPORATIONS * corporations) { config.SetCorporations( corporations); }
        void                SetStore(STORE * store) { config.SetStore(store); }
        void                SetStgSettings(const SETTINGS * s) { config.SetSettings(s); }
        void                SetSettings(const MODULE_SETTINGS & s) { settings = s; }
        int                 ParseSettings();

        int                 Start();
        int                 Stop();
        int                 Reload(const MODULE_SETTINGS & /*ms*/) { return 0; }
        bool                IsRunning() { return isRunning; }

        const std::string & GetStrError() const { return errorStr; }
        std::string         GetVersion() const { return "Stg Configurator v. 2.0"; }
        uint16_t            GetStartPosition() const { return 20; }
        uint16_t            GetStopPosition() const { return 20; }

    private:
        STG_CONFIG(const STG_CONFIG & rvalue);
        STG_CONFIG & operator=(const STG_CONFIG & rvalue);

        static void *       Run(void *);

        mutable std::string errorStr;
        STG_CONFIG_SETTINGS stgConfigSettings;
        pthread_t           thread;
        bool                nonstop;
        bool                isRunning;
        PLUGIN_LOGGER       logger;
        CONFIGPROTO         config;
        MODULE_SETTINGS     settings;
};
//-----------------------------------------------------------------------------

#endif
