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
$Revision: 1.31 $
$Date: 2010/10/07 20:04:48 $
$Author: faust $
*/


#ifndef USERS_H
#define USERS_H

#include <pthread.h>
#include <ctime>
#include <string>
#include <map>
#include <list>
#include <set>

#include "os_int.h"

#include "settings.h"
#include "user.h"
#include "tariffs.h"
#include "stg_logger.h"
#include "notifer.h"
#include "actions.h"
#include "noncopyable.h"
#include "eventloop.h"

const int userDeleteDelayTime = 120;

using namespace std;
class USERS;
//-----------------------------------------------------------------------------
class PROPERTY_NOTIFER_IP_BEFORE: public PROPERTY_NOTIFIER_BASE<uint32_t>
{
public:
    PROPERTY_NOTIFER_IP_BEFORE(USERS & us, user_iter u) : users(us), user(u) {};
    void        Notify(const uint32_t & oldValue, const uint32_t & newValue);
    user_iter   GetUser() const { return user; };
private:
    USERS &     users;
    user_iter   user;
};
//-----------------------------------------------------------------------------
class PROPERTY_NOTIFER_IP_AFTER: public PROPERTY_NOTIFIER_BASE<uint32_t>
{
public:
    PROPERTY_NOTIFER_IP_AFTER(USERS & us, user_iter u) : users(us), user(u) {};
    void        Notify(const uint32_t & oldValue, const uint32_t & newValue);
    user_iter   GetUser() const { return user; };
private:
    USERS &     users;
    user_iter   user;
};
//-----------------------------------------------------------------------------
struct USER_TO_DEL
{
USER_TO_DEL()
    : iter(),
      delTime(0)
{};

list<USER>::iterator iter;
time_t  delTime;
};
//-----------------------------------------------------------------------------
class USERS : private NONCOPYABLE
    {
    friend class PROPERTY_NOTIFER_IP_BEFORE;
    friend class PROPERTY_NOTIFER_IP_AFTER;

public:
    USERS(SETTINGS * s, BASE_STORE * store, TARIFFS * tariffs, const ADMIN & sysAdmin);
    ~USERS();

    int             FindByName(const string & login, user_iter * user) const;
    int             FindByID(int id, user_iter * user);

    bool            TariffInUse(const string & tariffName);

    void            AddNotifierUserAdd(NOTIFIER_BASE<user_iter> *);
    void            DelNotifierUserAdd(NOTIFIER_BASE<user_iter> *);

    void            AddNotifierUserDel(NOTIFIER_BASE<user_iter> *);
    void            DelNotifierUserDel(NOTIFIER_BASE<user_iter> *);

    int             Add(const string & login, const ADMIN & admin);
    void            Del(const string & login, const ADMIN & admin);

    int             ReadUsers();
    int             GetUserNum();

    int             FindByIPIdx(uint32_t ip, user_iter * user);

    int             OpenSearch();
    int             SearchNext(int handler, user_iter * u);
    int             CloseSearch(int handler);

    int             Start();
    int             Stop();

private:
    void            AddToIPIdx(user_iter);
    void            DelFromIPIdx(uint32_t ip);

    int             FindByNameNonLock(const string & login, user_iter * user) const;
    int             FindByIDNonLock(int id, user_iter * user);

    void            RealDelUser();
    void            ProcessActions();

    void            SetUserNotifiers(user_iter user);
    void            UnSetUserNotifiers(user_iter user);

    void            AddUserIntoIndexes(user_iter user);
    void            DelUserFromIndexes(user_iter user);

    static void *   Run(void *);
    void            NewMinute(const struct tm * t);
    void            NewDay(const struct tm * t);
    void            DayResetTraff(const struct tm * t);

    bool            TimeToWriteDetailStat(const struct tm * t);

    list<USER>          users;
    list<USER_TO_DEL>   usersToDelete;
    list<PROPERTY_NOTIFER_IP_BEFORE> userIPNotifiersBefore;
    list<PROPERTY_NOTIFER_IP_AFTER>  userIPNotifiersAfter;

    map<uint32_t, user_iter> ipIndex;
    map<string,   user_iter> loginIndex;

    SETTINGS *          settings;
    TARIFFS *           tariffs;
    BASE_STORE *        store;
    const ADMIN         sysAdmin;
    STG_LOGGER &        WriteServLog;

    bool                nonstop;
    bool                isRunning;

    mutable pthread_mutex_t mutex;
    pthread_t               thread;
    mutable unsigned int    handle;

    mutable map<int, user_iter> searchDescriptors;

    set <NOTIFIER_BASE<user_iter>*> onAddNotifiers;
    set <NOTIFIER_BASE<user_iter>*> onDelNotifiers;
    };
//-----------------------------------------------------------------------------
inline
void PROPERTY_NOTIFER_IP_BEFORE::Notify(const uint32_t & oldValue,
                                        const uint32_t &)
{
if (!oldValue)
    return;

//EVENT_LOOP_SINGLETON::GetInstance().Enqueue(users, &USERS::DelFromIPIdx, oldValue);
// Using explicit call to assure that index is valid, because fast reconnect with delayed call can result in authorization error
users.DelFromIPIdx(oldValue);
}
//-----------------------------------------------------------------------------
inline
void PROPERTY_NOTIFER_IP_AFTER::Notify(const uint32_t &,
                                       const uint32_t & newValue)
{
if (!newValue)
    return;

//EVENT_LOOP_SINGLETON::GetInstance().Enqueue(users, &USERS::AddToIPIdx, user);
// Using explicit call to assure that index is valid, because fast reconnect with delayed call can result in authorization error
users.AddToIPIdx(user);
}
//-----------------------------------------------------------------------------
#endif

