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


#ifndef USERS_IMPL_H
#define USERS_IMPL_H

#include <pthread.h>

#include <ctime>
#include <string>
#include <map>
#include <list>
#include <set>

#include "stg/os_int.h"
#include "stg/store.h"
#include "stg/users.h"
#include "stg/user.h"
#include "stg/tariffs.h"
#include "stg/logger.h"
#include "stg/notifer.h"
#include "stg/noncopyable.h"
#include "actions.h"
#include "eventloop.h"
#include "settings_impl.h"
#include "user_impl.h"

const int userDeleteDelayTime = 120;

typedef std::list<USER_IMPL>::iterator user_iter;
typedef std::list<USER_IMPL>::const_iterator const_user_iter;

class USERS_IMPL;
//-----------------------------------------------------------------------------
struct USER_TO_DEL {
USER_TO_DEL()
    : iter(),
      delTime(0)
{}

std::list<USER_IMPL>::iterator iter;
time_t  delTime;
};
//-----------------------------------------------------------------------------
class USERS_IMPL : private NONCOPYABLE, public USERS {
    friend class PROPERTY_NOTIFER_IP_BEFORE;
    friend class PROPERTY_NOTIFER_IP_AFTER;

public:
    USERS_IMPL(SETTINGS_IMPL * s, STORE * store, TARIFFS * tariffs, const ADMIN * sysAdmin);
    virtual ~USERS_IMPL();

    int             FindByName(const std::string & login, USER_PTR * user);

    bool            TariffInUse(const std::string & tariffName) const;

    void            AddNotifierUserAdd(NOTIFIER_BASE<USER_PTR> *);
    void            DelNotifierUserAdd(NOTIFIER_BASE<USER_PTR> *);

    void            AddNotifierUserDel(NOTIFIER_BASE<USER_PTR> *);
    void            DelNotifierUserDel(NOTIFIER_BASE<USER_PTR> *);

    void            AddNotifierUserAdd(NOTIFIER_BASE<USER_IMPL_PTR> *);
    void            DelNotifierUserAdd(NOTIFIER_BASE<USER_IMPL_PTR> *);

    void            AddNotifierUserDel(NOTIFIER_BASE<USER_IMPL_PTR> *);
    void            DelNotifierUserDel(NOTIFIER_BASE<USER_IMPL_PTR> *);

    int             Add(const std::string & login, const ADMIN * admin);
    void            Del(const std::string & login, const ADMIN * admin);

    bool            Authorize(const std::string & login, uint32_t ip,
                              uint32_t enabledDirs, const AUTH * auth);
    bool            Unauthorize(const std::string & login, const AUTH * auth);

    int             ReadUsers();
    size_t          Count() const { return users.size(); }

    int             FindByIPIdx(uint32_t ip, USER_PTR * user) const;
    int             FindByIPIdx(uint32_t ip, USER_IMPL ** user) const;
    bool            IsIPInIndex(uint32_t ip) const;

    int             OpenSearch();
    int             SearchNext(int handler, USER_PTR * user);
    int             SearchNext(int handler, USER_IMPL ** user);
    int             CloseSearch(int handler);

    int             Start();
    int             Stop();

private:
    USERS_IMPL(const USERS_IMPL & rvalue);
    USERS_IMPL & operator=(const USERS_IMPL & rvalue);

    void            AddToIPIdx(user_iter user);
    void            DelFromIPIdx(uint32_t ip);
    bool            FindByIPIdx(uint32_t ip, user_iter & iter) const;

    int             FindByNameNonLock(const std::string & login, user_iter * user);

    void            RealDelUser();
    void            ProcessActions();

    void            AddUserIntoIndexes(user_iter user);
    void            DelUserFromIndexes(user_iter user);

    static void *   Run(void *);
    void            NewMinute(const struct tm & t);
    void            NewDay(const struct tm & t);
    void            DayResetTraff(const struct tm & t);

    bool            TimeToWriteDetailStat(const struct tm & t);

    std::list<USER_IMPL>                  users;
    std::list<USER_TO_DEL>                usersToDelete;

    std::map<uint32_t, user_iter>         ipIndex;
    std::map<std::string, user_iter>      loginIndex;

    SETTINGS_IMPL *     settings;
    TARIFFS *           tariffs;
    STORE *             store;
    const ADMIN *       sysAdmin;
    STG_LOGGER &        WriteServLog;

    bool                nonstop;
    bool                isRunning;

    mutable pthread_mutex_t mutex;
    pthread_t               thread;
    mutable unsigned int    handle;

    mutable std::map<int, user_iter>  searchDescriptors;

    std::set<NOTIFIER_BASE<USER_PTR>*> onAddNotifiers;
    std::set<NOTIFIER_BASE<USER_PTR>*> onDelNotifiers;
    std::set<NOTIFIER_BASE<USER_IMPL_PTR>*> onAddNotifiersImpl;
    std::set<NOTIFIER_BASE<USER_IMPL_PTR>*> onDelNotifiersImpl;
};
//-----------------------------------------------------------------------------
/*inline
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
}*/
//-----------------------------------------------------------------------------
#endif
