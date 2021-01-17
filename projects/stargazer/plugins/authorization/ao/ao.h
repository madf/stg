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
#include "stg/notifer.h"
#include "stg/user_ips.h"
#include "stg/user.h"
#include "stg/logger.h"

#include <string>
#include <vector>
#include <list>

#include <pthread.h>

namespace STG
{
struct Users;
}

class AUTH_AO;

using UserPtr = STG::User*;
using ConstUserPtr = const STG::User*;

template <typename T>
class CHG_BEFORE_NOTIFIER : public STG::PropertyNotifierBase<T> {
public:
                CHG_BEFORE_NOTIFIER(AUTH_AO & a, UserPtr u)
                    : user(u), auth(a) {}
                CHG_BEFORE_NOTIFIER(const CHG_BEFORE_NOTIFIER<T> & rvalue)
                    : user(rvalue.user), auth(rvalue.auth) {}
    void        Notify(const T & oldValue, const T & newValue);
    UserPtr    GetUser() const { return user; }

private:
    CHG_BEFORE_NOTIFIER<T> & operator=(const CHG_BEFORE_NOTIFIER<T> & rvalue);

    UserPtr        user;
    const AUTH_AO & auth;
};
//-----------------------------------------------------------------------------
template <typename T>
class CHG_AFTER_NOTIFIER : public STG::PropertyNotifierBase<T> {
public:
                CHG_AFTER_NOTIFIER(AUTH_AO & a, UserPtr u)
                    : user(u), auth(a) {}
                CHG_AFTER_NOTIFIER(const CHG_AFTER_NOTIFIER<T> & rvalue)
                    : user(rvalue.user), auth(rvalue.auth) {}
    void        Notify(const T & oldValue, const T & newValue);
    UserPtr    GetUser() const { return user; }

private:
    CHG_AFTER_NOTIFIER<T> & operator=(const CHG_AFTER_NOTIFIER<T> & rvalue);

    UserPtr        user;
    const AUTH_AO & auth;
};
//-----------------------------------------------------------------------------
class AUTH_AO : public STG::Auth {
public:
    AUTH_AO();

    void                SetUsers(STG::Users * u) override { users = u; }

    int                 Start() override;
    int                 Stop() override;
    int                 Reload(const STG::ModuleSettings & /*ms*/) override { return 0; }
    bool                IsRunning() override { return isRunning; }
    void                SetSettings(const STG::ModuleSettings &) override {}
    int                 ParseSettings() override { return 0; }
    const std::string & GetStrError() const override { return errorStr; }
    std::string         GetVersion() const override;
    uint16_t            GetStartPosition() const override { return 30; }
    uint16_t            GetStopPosition() const override { return 30; }

    int                 SendMessage(const STG::Message & msg, uint32_t ip) const override;

private:
    AUTH_AO(const AUTH_AO & rvalue);
    AUTH_AO & operator=(const AUTH_AO & rvalue);

    void                AddUser(UserPtr u);
    void                DelUser(UserPtr u);

    void                GetUsers();
    void                SetUserNotifiers(UserPtr u);
    void                UnSetUserNotifiers(UserPtr u);
    void                UpdateUserAuthorization(ConstUserPtr u) const;

    mutable std::string errorStr;
    STG::Users *             users;
    std::vector<UserPtr> userList;
    bool                isRunning;
    STG::ModuleSettings     settings;

    std::list<CHG_BEFORE_NOTIFIER<int> >      BeforeChgAONotifierList;
    std::list<CHG_AFTER_NOTIFIER<int> >       AfterChgAONotifierList;

    std::list<CHG_BEFORE_NOTIFIER<STG::UserIPs> > BeforeChgIPNotifierList;
    std::list<CHG_AFTER_NOTIFIER<STG::UserIPs> >  AfterChgIPNotifierList;

    class ADD_USER_NONIFIER: public STG::NotifierBase<UserPtr> {
    public:
        explicit ADD_USER_NONIFIER(AUTH_AO & a) : auth(a) {}
        virtual ~ADD_USER_NONIFIER() {}
        void Notify(const UserPtr & user) { auth.AddUser(user); }

    private:
        ADD_USER_NONIFIER(const ADD_USER_NONIFIER & rvalue);
        ADD_USER_NONIFIER & operator=(const ADD_USER_NONIFIER & rvalue);

        AUTH_AO & auth;
    } onAddUserNotifier;

    class DEL_USER_NONIFIER: public STG::NotifierBase<UserPtr> {
    public:
        explicit DEL_USER_NONIFIER(AUTH_AO & a) : auth(a) {}
        virtual ~DEL_USER_NONIFIER() {}
        void Notify(const UserPtr & user) { auth.DelUser(user); }

    private:
        DEL_USER_NONIFIER(const DEL_USER_NONIFIER & rvalue);
        DEL_USER_NONIFIER & operator=(const DEL_USER_NONIFIER & rvalue);

        AUTH_AO & auth;
    } onDelUserNotifier;

    STG::PluginLogger logger;

    friend class CHG_BEFORE_NOTIFIER<int>;
    friend class CHG_AFTER_NOTIFIER<int>;
    friend class CHG_BEFORE_NOTIFIER<STG::UserIPs>;
    friend class CHG_AFTER_NOTIFIER<STG::UserIPs>;

};
