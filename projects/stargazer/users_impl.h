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

#include <pthread.h>

#include <string>
#include <map>
#include <list>
#include <set>
#include <ctime>
#include <cstdint>

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

namespace STG
{

const int userDeleteDelayTime = 120;

typedef std::list<UserImpl>::iterator user_iter;
typedef std::list<UserImpl>::const_iterator const_user_iter;

class UsersImpl;
//-----------------------------------------------------------------------------
struct USER_TO_DEL {
USER_TO_DEL()
    : iter(),
      delTime(0)
{}

std::list<UserImpl>::iterator iter;
time_t  delTime;
};
//-----------------------------------------------------------------------------
class UsersImpl : public Users {
    friend class PROPERTY_NOTIFER_IP_BEFORE;
    friend class PROPERTY_NOTIFER_IP_AFTER;

public:
    using UserImplPtr = UserImpl*;

    UsersImpl(SettingsImpl * s, Store * store,
              Tariffs * tariffs, Services & svcs,
              const Admin& sysAdmin);
    virtual ~UsersImpl();

    int             FindByName(const std::string & login, UserPtr * user) override;
    int             FindByName(const std::string & login, ConstUserPtr * user) const override;
    bool            Exists(const std::string & login) const override;

    bool            TariffInUse(const std::string & tariffName) const override;

    void            AddNotifierUserAdd(NotifierBase<UserPtr> *) override;
    void            DelNotifierUserAdd(NotifierBase<UserPtr> *) override;

    void            AddNotifierUserDel(NotifierBase<UserPtr> *) override;
    void            DelNotifierUserDel(NotifierBase<UserPtr> *) override;

    void            AddNotifierUserAdd(NotifierBase<UserImplPtr> *);
    void            DelNotifierUserAdd(NotifierBase<UserImplPtr> *);

    void            AddNotifierUserDel(NotifierBase<UserImplPtr> *);
    void            DelNotifierUserDel(NotifierBase<UserImplPtr> *);

    int             Add(const std::string & login, const Admin * admin) override;
    void            Del(const std::string & login, const Admin * admin) override;

    bool            Authorize(const std::string & login, uint32_t ip,
                              uint32_t enabledDirs, const Auth * auth) override;
    bool            Unauthorize(const std::string & login,
                                const Auth * auth,
                                const std::string & reason) override;

    int             ReadUsers() override;
    size_t          Count() const override { return users.size(); }

    int             FindByIPIdx(uint32_t ip, UserPtr * user) const override;
    int             FindByIPIdx(uint32_t ip, UserImpl ** user) const;
    bool            IsIPInIndex(uint32_t ip) const override;
    bool            IsIPInUse(uint32_t ip, const std::string & login, ConstUserPtr * user) const override;

    int             OpenSearch() override;
    int             SearchNext(int handler, UserPtr * user) override;
    int             SearchNext(int handler, UserImpl ** user);
    int             CloseSearch(int handler) override;

    int             Start() override;
    int             Stop() override;

private:
    UsersImpl(const UsersImpl & rvalue);
    UsersImpl & operator=(const UsersImpl & rvalue);

    void            AddToIPIdx(user_iter user);
    void            DelFromIPIdx(uint32_t ip);
    bool            FindByIPIdx(uint32_t ip, user_iter & iter) const;

    int             FindByNameNonLock(const std::string & login, user_iter * user);
    int             FindByNameNonLock(const std::string & login, const_user_iter * user) const;

    void            RealDelUser();
    void            ProcessActions();

    void            AddUserIntoIndexes(user_iter user);
    void            DelUserFromIndexes(user_iter user);

    static void *   Run(void *);
    void            NewMinute(const struct tm & t);
    void            NewDay(const struct tm & t);
    void            DayResetTraff(const struct tm & t);

    bool            TimeToWriteDetailStat(const struct tm & t);

    std::list<UserImpl>                  users;
    std::list<USER_TO_DEL>                usersToDelete;

    std::map<uint32_t, user_iter>         ipIndex;
    std::map<std::string, user_iter>      loginIndex;

    SettingsImpl *     settings;
    Tariffs *           tariffs;
    Services &          m_services;
    Store *             store;
    const Admin&       sysAdmin;
    Logger &        WriteServLog;

    bool                nonstop;
    bool                isRunning;

    mutable pthread_mutex_t mutex;
    pthread_t               thread;
    mutable unsigned int    handle;

    mutable std::map<int, user_iter>  searchDescriptors;

    std::set<NotifierBase<UserPtr>*> onAddNotifiers;
    std::set<NotifierBase<UserPtr>*> onDelNotifiers;
    std::set<NotifierBase<UserImplPtr>*> onAddNotifiersImpl;
    std::set<NotifierBase<UserImplPtr>*> onDelNotifiersImpl;
};

}
