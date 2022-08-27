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

#include "stg/auth.h"
#include "stg/module_settings.h"
#include "stg/store.h"
#include "stg/subscriptions.h"
#include "stg/user_ips.h"
#include "stg/user.h"
#include "stg/logger.h"

#include <string>
#include <vector>
#include <list>

#include <pthread.h>

namespace STG
{
class Users;

using UserPtr = User*;
using ConstUserPtr = const User*;
//-----------------------------------------------------------------------------
class AUTH_AO : public Auth
{
    public:
        AUTH_AO();

        void                SetUsers(Users * u) override { users = u; }

        int                 Start() override;
        int                 Stop() override;
        int                 Reload(const ModuleSettings & /*ms*/) override { return 0; }
        bool                IsRunning() override { return isRunning; }
        void                SetSettings(const ModuleSettings &) override {}
        int                 ParseSettings() override { return 0; }
        const std::string & GetStrError() const override { return errorStr; }
        std::string         GetVersion() const override;
        uint16_t            GetStartPosition() const override { return 30; }
        uint16_t            GetStopPosition() const override { return 30; }

        int                 SendMessage(const Message & msg, uint32_t ip) const override;

    private:
        AUTH_AO(const AUTH_AO & rvalue);
        AUTH_AO & operator=(const AUTH_AO & rvalue);

        void                AddUser(UserPtr u);
        void                DelUser(UserPtr u);

        void                GetUsers();
        void                SetUserNotifiers(UserPtr u);
        void                UnSetUserNotifiers(UserPtr u);
        void                UpdateUserAuthorization(ConstUserPtr u) const;
        void                Unauthorize(UserPtr u);

        mutable std::string errorStr;
        Users *             users;
        std::vector<UserPtr> userList;
        bool                isRunning;
        ModuleSettings     settings;

        using ConnHolder = std::tuple<int, ScopedConnection, ScopedConnection, ScopedConnection, ScopedConnection>;
        std::vector<ConnHolder> m_conns;

        ScopedConnection m_onAddUserConn;
        ScopedConnection m_onDelUserConn;

        PluginLogger logger;
};

}
