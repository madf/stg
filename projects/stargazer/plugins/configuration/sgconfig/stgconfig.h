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

#pragma once

#include "configproto.h"

#include "stg/plugin.h"
#include "stg/logger.h"

#include <string>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop

class STG_CONFIG_SETTINGS
{
    public:
        STG_CONFIG_SETTINGS() : m_port(0), m_bindAddress("0.0.0.0") {}
        const std::string & GetStrError() const { return errorStr; }
        bool ParseSettings(const STG::ModuleSettings & s);
        uint16_t GetPort() const { return m_port; }
        const std::string & GetBindAddress() const { return m_bindAddress; }
    private:
        std::string errorStr;
        uint16_t m_port;
        std::string m_bindAddress;
};

class STG_CONFIG : public STG::Plugin
{
    public:
        STG_CONFIG();

        void                SetUsers(STG::Users * users) override { config.SetUsers(users); }
        void                SetTariffs(STG::Tariffs * tariffs) override { config.SetTariffs(tariffs); }
        void                SetAdmins(STG::Admins * admins) override { config.SetAdmins(admins); }
        void                SetServices(STG::Services * services) override { config.SetServices(services); }
        void                SetCorporations(STG::Corporations * corporations) override { config.SetCorporations( corporations); }
        void                SetStore(STG::Store * store) override { config.SetStore(store); }
        void                SetStgSettings(const STG::Settings * s) override { config.SetSettings(s); }
        void                SetSettings(const STG::ModuleSettings & s) override { settings = s; }
        int                 ParseSettings() override;

        int                 Start() override;
        int                 Stop() override;
        int                 Reload(const STG::ModuleSettings & /*ms*/) override { return 0; }
        bool                IsRunning() override { return isRunning; }

        const std::string & GetStrError() const override { return errorStr; }
        std::string         GetVersion() const override { return "Stg Configurator v. 2.0"; }
        uint16_t            GetStartPosition() const override { return 20; }
        uint16_t            GetStopPosition() const override { return 20; }

    private:
        STG_CONFIG(const STG_CONFIG & rvalue);
        STG_CONFIG & operator=(const STG_CONFIG & rvalue);

        void                Run(std::stop_token token);

        mutable std::string errorStr;
        STG_CONFIG_SETTINGS stgConfigSettings;
        std::jthread        m_thread;
        bool                nonstop;
        bool                isRunning;
        STG::PluginLogger   logger;
        CONFIGPROTO         config;
        STG::ModuleSettings settings;
};
