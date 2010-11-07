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

#include <string>
#include <pthread.h>
#include "base_auth.h"
#include "notifer.h"
#include "user_ips.h"
#include "../../../users.h"

using namespace std;

extern "C" BASE_PLUGIN * GetPlugin();

class AUTH_STRESS;
//-----------------------------------------------------------------------------
template <typename varParamType>
class CHG_BEFORE_NOTIFIER: public PROPERTY_NOTIFIER_BASE<varParamType>
{
public:
    void Notify(const varParamType & oldValue, const varParamType & newValue);
    void        SetUser(user_iter u) { user = u; }
    user_iter   GetUser() {return user; }
    void        SetAuthorizator(const AUTH_STRESS * a) { auth = a; }

private:
    user_iter   user;
    const       AUTH_STRESS * auth;
};
//-----------------------------------------------------------------------------
template <typename varParamType>
class CHG_AFTER_NOTIFIER: public PROPERTY_NOTIFIER_BASE<varParamType>
{
public:
    void        Notify(const varParamType & oldValue, const varParamType & newValue);
    void        SetUser(user_iter u) { user = u; }
    user_iter   GetUser() {return user; }
    void        SetAuthorizator(const AUTH_STRESS * a) { auth = a; }

private:
    user_iter   user;
    const AUTH_STRESS * auth;
};
//-----------------------------------------------------------------------------
class AUTH_STRESS_SETTINGS
{
public:
                    AUTH_STRESS_SETTINGS();
    const string&   GetStrError() const { return errorStr; }
    int             ParseSettings(const MODULE_SETTINGS & s);
    int             GetAverageOnlineTime() const;
private:
    int             ParseIntInRange(const string & str, int min, int max, int * val);
    int             averageOnlineTime;
    string          errorStr;
};
//-----------------------------------------------------------------------------
class AUTH_STRESS :public BASE_AUTH
{
public:
    AUTH_STRESS();
    virtual ~AUTH_STRESS(){};

    void                SetUsers(USERS * u);
    void                SetTariffs(TARIFFS * t){};
    void                SetAdmins(ADMINS * a){};
    void                SetTraffcounter(TRAFFCOUNTER * tc){};
    void                SetStore(BASE_STORE * ){};
    void                SetStgSettings(const SETTINGS *){};

    int                 Start();
    int                 Stop();
    bool                IsRunning();
    void                SetSettings(const MODULE_SETTINGS & s);
    int                 ParseSettings();
    const string      & GetStrError() const;
    const string        GetVersion() const;
    uint16_t            GetStartPosition() const;
    uint16_t            GetStopPosition() const;

    void                AddUser(user_iter u);
    void                DelUser(user_iter u);

    void                Authorize(user_iter u) const;
    void                Unauthorize(user_iter u) const;

    int                 SendMessage(const STG_MSG & msg, uint32_t ip) const;

private:
    void                GetUsers();
    void                SetUserNotifiers(user_iter u);
    void                UnSetUserNotifiers(user_iter u);

    bool                nonstop;

    static void *       Run(void *);

    mutable string      errorStr;
    AUTH_STRESS_SETTINGS    stressSettings;
    USERS             * users;
    list<user_iter>     usersList;
    bool                isRunning;
    MODULE_SETTINGS     settings;

    pthread_t           thread;
    pthread_mutex_t     mutex;

    list<CHG_BEFORE_NOTIFIER<USER_IPS> >    BeforeChgIPNotifierList;
    list<CHG_AFTER_NOTIFIER<USER_IPS> >     AfterChgIPNotifierList;

    class ADD_USER_NONIFIER: public NOTIFIER_BASE<user_iter>
    {
    public:
        ADD_USER_NONIFIER(){};
        virtual ~ADD_USER_NONIFIER(){};

        void SetAuthorizator(AUTH_STRESS * a) { auth = a; }
        void Notify(const user_iter & user)
            {
            auth->AddUser(user);
            }

    private:
        AUTH_STRESS * auth;
    } onAddUserNotifier;

    class DEL_USER_NONIFIER: public NOTIFIER_BASE<user_iter>
    {
    public:
        DEL_USER_NONIFIER(){};
        virtual ~DEL_USER_NONIFIER(){};

        void SetAuthorizator(AUTH_STRESS * a) { auth = a; }
        void Notify(const user_iter & user)
            {
            auth->DelUser(user);
            }

    private:
        AUTH_STRESS * auth;
    } onDelUserNotifier;

};
//-----------------------------------------------------------------------------

#endif


