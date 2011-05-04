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
 $Revision: 1.3 $
 $Date: 2009/06/19 12:50:32 $
 $Author: faust $
 */


#ifndef STRESS_H
#define STRESS_H

#include <pthread.h>

#include <string>
#include <list>

#include "stg/auth.h"
#include "stg/notifer.h"
#include "stg/user_ips.h"
#include "stg/users.h"
#include "stg/module_settings.h"

extern "C" PLUGIN * GetPlugin();

class AUTH_STRESS;
//-----------------------------------------------------------------------------
template <typename varParamType>
class CHG_BEFORE_NOTIFIER: public PROPERTY_NOTIFIER_BASE<varParamType> {
public:
    void Notify(const varParamType & oldValue, const varParamType & newValue);
    void        SetUser(USER_PTR u) { user = u; }
    USER_PTR    GetUser() {return user; }
    void        SetAuthorizator(const AUTH_STRESS * a) { auth = a; }

private:
    USER_PTR    user;
    const       AUTH_STRESS * auth;
};
//-----------------------------------------------------------------------------
template <typename varParamType>
class CHG_AFTER_NOTIFIER: public PROPERTY_NOTIFIER_BASE<varParamType> {
public:
    void        Notify(const varParamType & oldValue, const varParamType & newValue);
    void        SetUser(USER_PTR u) { user = u; }
    USER_PTR    GetUser() {return user; }
    void        SetAuthorizator(const AUTH_STRESS * a) { auth = a; }

private:
    USER_PTR   user;
    const AUTH_STRESS * auth;
};
//-----------------------------------------------------------------------------
class AUTH_STRESS_SETTINGS {
public:
                    AUTH_STRESS_SETTINGS();
    const std::string & GetStrError() const { return errorStr; }
    int             ParseSettings(const MODULE_SETTINGS & s);
    int             GetAverageOnlineTime() const;
private:
    int             ParseIntInRange(const std::string & str, int min, int max, int * val);
    int             averageOnlineTime;
    std::string     errorStr;
};
//-----------------------------------------------------------------------------
class AUTH_STRESS :public AUTH {
friend class CHG_BEFORE_NOTIFIER<USER_IPS>;
friend class CHG_AFTER_NOTIFIER<USER_IPS>;
public:
    AUTH_STRESS();
    virtual ~AUTH_STRESS() {}

    void                SetUsers(USERS * u);
    void                SetTariffs(TARIFFS *) {}
    void                SetAdmins(ADMINS *) {}
    void                SetTraffcounter(TRAFFCOUNTER *) {}
    void                SetStore(STORE *) {}
    void                SetStgSettings(const SETTINGS *) {}

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; }
    bool                IsRunning();
    void                SetSettings(const MODULE_SETTINGS & s);
    int                 ParseSettings();
    const std::string & GetStrError() const;
    const std::string   GetVersion() const;
    uint16_t            GetStartPosition() const;
    uint16_t            GetStopPosition() const;

    int                 SendMessage(const STG_MSG & msg, uint32_t ip) const;

private:
    void                GetUsers();
    void                SetUserNotifiers(USER_PTR u);
    void                UnSetUserNotifiers(USER_PTR u);

    void                AddUser(USER_PTR u);
    void                DelUser(USER_PTR u);

    void                Authorize(USER_PTR u) const;
    void                Unauthorize(USER_PTR u) const;

    static void *       Run(void *);

    bool                nonstop;

    mutable std::string errorStr;
    AUTH_STRESS_SETTINGS stressSettings;
    USERS             * users;
    std::list<USER_PTR> usersList;
    bool                isRunning;
    MODULE_SETTINGS     settings;

    pthread_t           thread;
    pthread_mutex_t     mutex;

    std::list<CHG_BEFORE_NOTIFIER<USER_IPS> > BeforeChgIPNotifierList;
    std::list<CHG_AFTER_NOTIFIER<USER_IPS> > AfterChgIPNotifierList;

    class ADD_USER_NONIFIER: public NOTIFIER_BASE<USER_PTR> {
    public:
        ADD_USER_NONIFIER() {}
        virtual ~ADD_USER_NONIFIER() {}

        void SetAuthorizator(AUTH_STRESS * a) { auth = a; }
        void Notify(const USER_PTR & user)
            {
            auth->AddUser(user);
            }

    private:
        AUTH_STRESS * auth;
    } onAddUserNotifier;

    class DEL_USER_NONIFIER: public NOTIFIER_BASE<USER_PTR> {
    public:
        DEL_USER_NONIFIER() {}
        virtual ~DEL_USER_NONIFIER() {}

        void SetAuthorizator(AUTH_STRESS * a) { auth = a; }
        void Notify(const USER_PTR & user)
            {
            auth->DelUser(user);
            }

    private:
        AUTH_STRESS * auth;
    } onDelUserNotifier;

};
//-----------------------------------------------------------------------------

#endif
