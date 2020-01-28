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

/*
 $Revision: 1.21 $
 $Date: 2010/09/10 06:38:26 $
 $Author: faust $
*/

#ifndef AO_H
#define AO_H

#include <pthread.h>

#include <string>
#include <vector>
#include <list>

#include "stg/auth.h"
#include "stg/store.h"
#include "stg/notifer.h"
#include "stg/user_ips.h"
#include "stg/user.h"
#include "stg/logger.h"

extern "C" PLUGIN * GetPlugin();

class AUTH_AO;
class USERS;
//-----------------------------------------------------------------------------
template <typename T>
class CHG_BEFORE_NOTIFIER : public PROPERTY_NOTIFIER_BASE<T> {
public:
                CHG_BEFORE_NOTIFIER(AUTH_AO & a, USER_PTR u)
                    : PROPERTY_NOTIFIER_BASE<T>(), user(u), auth(a) {}
                CHG_BEFORE_NOTIFIER(const CHG_BEFORE_NOTIFIER<T> & rvalue)
                    : PROPERTY_NOTIFIER_BASE<T>(),
                      user(rvalue.user), auth(rvalue.auth) {}
    void        Notify(const T & oldValue, const T & newValue);
    USER_PTR    GetUser() const { return user; }

private:
    CHG_BEFORE_NOTIFIER<T> & operator=(const CHG_BEFORE_NOTIFIER<T> & rvalue);

    USER_PTR        user;
    const AUTH_AO & auth;
};
//-----------------------------------------------------------------------------
template <typename T>
class CHG_AFTER_NOTIFIER : public PROPERTY_NOTIFIER_BASE<T> {
public:
                CHG_AFTER_NOTIFIER(AUTH_AO & a, USER_PTR u)
                    : PROPERTY_NOTIFIER_BASE<T>(), user(u), auth(a) {}
                CHG_AFTER_NOTIFIER(const CHG_AFTER_NOTIFIER<T> & rvalue)
                    : PROPERTY_NOTIFIER_BASE<T>(),
                      user(rvalue.user), auth(rvalue.auth) {}
    void        Notify(const T & oldValue, const T & newValue);
    USER_PTR    GetUser() const { return user; }

private:
    CHG_AFTER_NOTIFIER<T> & operator=(const CHG_AFTER_NOTIFIER<T> & rvalue);

    USER_PTR        user;
    const AUTH_AO & auth;
};
//-----------------------------------------------------------------------------
class AUTH_AO : public AUTH {
public:
    AUTH_AO();
    virtual ~AUTH_AO(){}

    void                SetUsers(USERS * u) { users = u; }

    int                 Start();
    int                 Stop();
    int                 Reload(const MODULE_SETTINGS & /*ms*/) { return 0; }
    bool                IsRunning() { return isRunning; }
    void                SetSettings(const MODULE_SETTINGS &) {}
    int                 ParseSettings() { return 0; }
    const std::string & GetStrError() const { return errorStr; }
    std::string         GetVersion() const;
    uint16_t            GetStartPosition() const { return 30; }
    uint16_t            GetStopPosition() const { return 30; }

    void                AddUser(USER_PTR u);
    void                DelUser(USER_PTR u);

    int                 SendMessage(const STG_MSG & msg, uint32_t ip) const;

private:
    AUTH_AO(const AUTH_AO & rvalue);
    AUTH_AO & operator=(const AUTH_AO & rvalue);

    void                GetUsers();
    void                SetUserNotifiers(USER_PTR u);
    void                UnSetUserNotifiers(USER_PTR u);
    void                UpdateUserAuthorization(CONST_USER_PTR u) const;

    mutable std::string errorStr;
    USERS *             users;
    std::vector<USER_PTR> userList;
    bool                isRunning;
    MODULE_SETTINGS     settings;

    std::list<CHG_BEFORE_NOTIFIER<int> >      BeforeChgAONotifierList;
    std::list<CHG_AFTER_NOTIFIER<int> >       AfterChgAONotifierList;

    std::list<CHG_BEFORE_NOTIFIER<USER_IPS> > BeforeChgIPNotifierList;
    std::list<CHG_AFTER_NOTIFIER<USER_IPS> >  AfterChgIPNotifierList;

    class ADD_USER_NONIFIER: public NOTIFIER_BASE<USER_PTR> {
    public:
        explicit ADD_USER_NONIFIER(AUTH_AO & a) : auth(a) {}
        virtual ~ADD_USER_NONIFIER() {}
        void Notify(const USER_PTR & user) { auth.AddUser(user); }

    private:
        ADD_USER_NONIFIER(const ADD_USER_NONIFIER & rvalue);
        ADD_USER_NONIFIER & operator=(const ADD_USER_NONIFIER & rvalue);

        AUTH_AO & auth;
    } onAddUserNotifier;

    class DEL_USER_NONIFIER: public NOTIFIER_BASE<USER_PTR> {
    public:
        explicit DEL_USER_NONIFIER(AUTH_AO & a) : auth(a) {}
        virtual ~DEL_USER_NONIFIER() {}
        void Notify(const USER_PTR & user) { auth.DelUser(user); }

    private:
        DEL_USER_NONIFIER(const DEL_USER_NONIFIER & rvalue);
        DEL_USER_NONIFIER & operator=(const DEL_USER_NONIFIER & rvalue);

        AUTH_AO & auth;
    } onDelUserNotifier;
    PLUGIN_LOGGER logger;

    friend class CHG_BEFORE_NOTIFIER<int>;
    friend class CHG_AFTER_NOTIFIER<int>;
    friend class CHG_BEFORE_NOTIFIER<USER_IPS>;
    friend class CHG_AFTER_NOTIFIER<USER_IPS>;

};
//-----------------------------------------------------------------------------

#endif
