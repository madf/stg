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

#include <string>
#include <pthread.h>
#include "base_auth.h"
#include "base_store.h"
#include "notifer.h"
#include "user_ips.h"
#include "../../../users.h"

using namespace std;

extern "C" BASE_PLUGIN * GetPlugin();

class AUTH_AO;
//-----------------------------------------------------------------------------
template <typename varParamType>
class CHG_BEFORE_NOTIFIER: public PROPERTY_NOTIFIER_BASE<varParamType>
{
public:
                CHG_BEFORE_NOTIFIER(AUTH_AO & a, user_iter u) : user(u), auth(a) {}
    void        Notify(const varParamType & oldValue, const varParamType & newValue);
    //void        SetUser(user_iter u) { user = u; }
    user_iter   GetUser() {return user; }
    //void        SetAuthorizator(const AUTH_AO * a) { auth = a; }

private:
    user_iter   user;
    const       AUTH_AO & auth;
};
//-----------------------------------------------------------------------------
template <typename varParamType>
class CHG_AFTER_NOTIFIER: public PROPERTY_NOTIFIER_BASE<varParamType>
{
public:
                CHG_AFTER_NOTIFIER(AUTH_AO & a, user_iter u) : user(u), auth(a) {}
    void        Notify(const varParamType & oldValue, const varParamType & newValue);
    //void        SetUser(user_iter u) { user = u; }
    user_iter   GetUser() {return user; }
    //void        SetAuthorizator(const AUTH_AO * a) { auth = a; }

private:
    user_iter   user;
    const AUTH_AO & auth;
};
//-----------------------------------------------------------------------------
class AUTH_AO_SETTINGS
{
public:
    const string& GetStrError() const { static string s; return s; }
    int ParseSettings(const MODULE_SETTINGS &) { return 0; }
};
//-----------------------------------------------------------------------------
class AUTH_AO :public BASE_AUTH
{
public:
    AUTH_AO();
    virtual ~AUTH_AO(){};

    void                SetUsers(USERS * u);
    void                SetTariffs(TARIFFS *){};
    void                SetAdmins(ADMINS *){};
    void                SetTraffcounter(TRAFFCOUNTER *){};
    void                SetStore(BASE_STORE *){};
    void                SetStgSettings(const SETTINGS *){};

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; };
    bool                IsRunning();
    void                SetSettings(const MODULE_SETTINGS & s);
    int                 ParseSettings();
    const string      & GetStrError() const;
    const string        GetVersion() const;
    uint16_t            GetStartPosition() const;
    uint16_t            GetStopPosition() const;

    void                AddUser(user_iter u);
    void                DelUser(user_iter u);

    void                UpdateUserAuthorization(user_iter u) const;
    void                Unauthorize(user_iter u) const;

    int                 SendMessage(const STG_MSG & msg, uint32_t ip) const;

private:
    void                GetUsers();
    void                SetUserNotifiers(user_iter u);
    void                UnSetUserNotifiers(user_iter u);

    mutable string      errorStr;
    AUTH_AO_SETTINGS    aoSettings;
    USERS             * users;
    list<user_iter>     usersList;
    bool                isRunning;
    MODULE_SETTINGS     settings;

    /*
    мы должны перепроверить возможность авторизации юзера при изменении
    следующих его параметров:
    - alwaysOnline
    - ips
    */

    list<CHG_BEFORE_NOTIFIER<int> >         BeforeChgAONotifierList;
    list<CHG_AFTER_NOTIFIER<int> >          AfterChgAONotifierList;

    list<CHG_BEFORE_NOTIFIER<USER_IPS> >    BeforeChgIPNotifierList;
    list<CHG_AFTER_NOTIFIER<USER_IPS> >     AfterChgIPNotifierList;

    class ADD_USER_NONIFIER: public NOTIFIER_BASE<user_iter>
    {
    public:
        ADD_USER_NONIFIER(AUTH_AO & a) : auth(a) {};
        virtual ~ADD_USER_NONIFIER(){};

        //void SetAuthorizator(AUTH_AO * a) { auth = a; }
        void Notify(const user_iter & user)
            {
            auth.AddUser(user);
            }

    private:
        AUTH_AO & auth;
    } onAddUserNotifier;

    class DEL_USER_NONIFIER: public NOTIFIER_BASE<user_iter>
    {
    public:
        DEL_USER_NONIFIER(AUTH_AO & a) : auth(a) {};
        virtual ~DEL_USER_NONIFIER(){};

        //void SetAuthorizator(AUTH_AO * a) { auth = a; }
        void Notify(const user_iter & user)
            {
            auth.DelUser(user);
            }

    private:
        AUTH_AO & auth;
    } onDelUserNotifier;

};
//-----------------------------------------------------------------------------

#endif


