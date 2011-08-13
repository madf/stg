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
#include <list>

#include "stg/auth.h"
#include "stg/store.h"
#include "stg/notifer.h"
#include "stg/user_ips.h"
#include "stg/user.h"

extern "C" PLUGIN * GetPlugin();

class AUTH_AO;
class USERS;
//-----------------------------------------------------------------------------
template <typename varParamType>
class CHG_BEFORE_NOTIFIER : public PROPERTY_NOTIFIER_BASE<varParamType> {
public:
                CHG_BEFORE_NOTIFIER(AUTH_AO & a, USER_PTR u) : user(u), auth(a) {}
    void        Notify(const varParamType & oldValue, const varParamType & newValue);
    USER_PTR    GetUser() const { return user; }

private:
    USER_PTR        user;
    const AUTH_AO & auth;
};
//-----------------------------------------------------------------------------
template <typename varParamType>
class CHG_AFTER_NOTIFIER : public PROPERTY_NOTIFIER_BASE<varParamType> {
public:
                CHG_AFTER_NOTIFIER(AUTH_AO & a, USER_PTR u) : user(u), auth(a) {}
    void        Notify(const varParamType & oldValue, const varParamType & newValue);
    USER_PTR    GetUser() const { return user; }

private:
    USER_PTR        user;
    const AUTH_AO & auth;
};
//-----------------------------------------------------------------------------
class AUTH_AO : public AUTH {
public:
    AUTH_AO();
    virtual ~AUTH_AO(){};

    void                SetUsers(USERS * u) { users = u; }

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; }
    bool                IsRunning() { return isRunning; }
    void                SetSettings(const MODULE_SETTINGS &) {}
    int                 ParseSettings() { return 0; }
    const std::string & GetStrError() const { return errorStr; }
    const std::string   GetVersion() const;
    uint16_t            GetStartPosition() const { return 70; }
    uint16_t            GetStopPosition() const { return 70; }

    void                AddUser(USER_PTR u);
    void                DelUser(USER_PTR u);

    void                UpdateUserAuthorization(USER_PTR u) const;
    void                Unauthorize(USER_PTR u) const;

    int                 SendMessage(const STG_MSG & msg, uint32_t ip) const;

private:
    void                GetUsers();
    void                SetUserNotifiers(USER_PTR u);
    void                UnSetUserNotifiers(USER_PTR u);

    mutable std::string errorStr;
    USERS *             users;
    std::list<USER_PTR> usersList;
    bool                isRunning;
    MODULE_SETTINGS     settings;

    list<CHG_BEFORE_NOTIFIER<int> >      BeforeChgAONotifierList;
    list<CHG_AFTER_NOTIFIER<int> >       AfterChgAONotifierList;

    list<CHG_BEFORE_NOTIFIER<USER_IPS> > BeforeChgIPNotifierList;
    list<CHG_AFTER_NOTIFIER<USER_IPS> >  AfterChgIPNotifierList;

    class ADD_USER_NONIFIER: public NOTIFIER_BASE<USER_PTR> {
    public:
        ADD_USER_NONIFIER(AUTH_AO & a) : auth(a) {}
        virtual ~ADD_USER_NONIFIER() {}

        void Notify(const USER_PTR & user)
            {
            auth.AddUser(user);
            }

    private:
        AUTH_AO & auth;
    } onAddUserNotifier;

    class DEL_USER_NONIFIER: public NOTIFIER_BASE<USER_PTR> {
    public:
        DEL_USER_NONIFIER(AUTH_AO & a) : auth(a) {}
        virtual ~DEL_USER_NONIFIER() {}

        void Notify(const USER_PTR & user)
            {
            auth.DelUser(user);
            }

    private:
        AUTH_AO & auth;
    } onDelUserNotifier;

};
//-----------------------------------------------------------------------------

#endif
