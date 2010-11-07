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
 *    Date: 27.10.2002
 */

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

/*
 $Revision: 1.61 $
 $Date: 2010/09/13 05:56:42 $
 $Author: faust $
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include <csignal>
#include <cassert>
#include <algorithm>
#include <utility>
#include <string>
#include <vector>

#include "settings.h"
#include "users.h"
#include "user.h"
#include "common.h"
#include "stg_timer.h"

using namespace std;

extern const volatile time_t stgTime;

//#define USERS_DEBUG 1

//-----------------------------------------------------------------------------
USERS::USERS(SETTINGS * s, BASE_STORE * st, TARIFFS * t, const ADMIN & sa)
    : users(),
      usersToDelete(),
      userIPNotifiersBefore(),
      userIPNotifiersAfter(),
      ipIndex(),
      loginIndex(),
      settings(s),
      tariffs(t),
      store(st),
      sysAdmin(sa),
      WriteServLog(GetStgLogger()),
      nonstop(false),
      isRunning(false),
      mutex(),
      thread(),
      handle(0),
      searchDescriptors(),
      onAddNotifiers(),
      onDelNotifiers()
{
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
pthread_mutex_init(&mutex, &attr);
}
//-----------------------------------------------------------------------------
USERS::~USERS()
{
pthread_mutex_destroy(&mutex);
}
//-----------------------------------------------------------------------------
int USERS::FindByNameNonLock(const string & login, user_iter * user) const
{
map<string, user_iter>::const_iterator iter;
iter = loginIndex.find(login);
if (iter != loginIndex.end())
    {
    if (user)
        *user = iter->second;
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int USERS::FindByName(const string & login, user_iter * user) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
return FindByNameNonLock(login, user);
}
//-----------------------------------------------------------------------------
bool USERS::TariffInUse(const string & tariffName)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
list<USER>::iterator iter;
iter = users.begin();
while (iter != users.end())
    {
    if (iter->property.tariffName.Get() == tariffName)
        return true;
    ++iter;
    }
return false;
}
//-----------------------------------------------------------------------------
int USERS::Add(const string & login, const ADMIN & admin)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
const PRIV * priv = admin.GetPriv();

if (!priv->userAddDel)
    {
    WriteServLog("%s tried to add user \'%s\'. Access denied.",
         admin.GetLogStr().c_str(), login.c_str());
    /*errorStr = "Admin \'" + admin.GetLogin() +
               "\': tried to add user \'" + ud->login + "\'. Access denied.";*/
    return -1;
    }

//////
if (store->AddUser(login))
    {
    //TODO
    //WriteServLog("Admin \'%s\': tried to add user \'%s\'. Access denied.",
    //     admin.GetLogin().c_str(), ud->login.c_str());
    return -1;
    }
//////

USER u(settings, store, tariffs, sysAdmin, &ipIndex);

struct tm * tms;
time_t t = stgTime;

tms = localtime(&t);

tms->tm_hour = 0;
tms->tm_min = 0;
tms->tm_sec = 0;

if (settings->GetDayResetTraff() > tms->tm_mday)
    tms->tm_mon -= 1;

tms->tm_mday = settings->GetDayResetTraff();

u.SetLogin(login);

u.SetPassiveTimeAsNewUser();

u.WriteConf();
u.WriteStat();

WriteServLog("%s User \'%s\' added.",
         admin.GetLogStr().c_str(), login.c_str());

u.OnAdd();

users.push_front(u);

AddUserIntoIndexes(users.begin());
SetUserNotifiers(users.begin());

// Уведомляем всех желающих, что добавлен новый пользователь
set<NOTIFIER_BASE<user_iter> *>::iterator ni = onAddNotifiers.begin();
while (ni != onAddNotifiers.end())
    {
    (*ni)->Notify(users.begin());
    ++ni;
    }

return 0;
}
//-----------------------------------------------------------------------------
void USERS::Del(const string & login, const ADMIN & admin)
{
const PRIV * priv = admin.GetPriv();
user_iter u;

if (!priv->userAddDel)
    {
    WriteServLog("%s tried to remove user \'%s\'. Access denied.",
         admin.GetLogStr().c_str(), login.c_str());
    return;
    }


    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);

    if (FindByNameNonLock(login, &u))
        {
        WriteServLog("%s tried to delete user \'%s\': not found.",
                     admin.GetLogStr().c_str(),
                     login.c_str());
        return;
        }
    }

set<NOTIFIER_BASE<user_iter> *>::iterator ni = onDelNotifiers.begin();
while (ni != onDelNotifiers.end())
    {
    (*ni)->Notify(u);
    ++ni;
    }

    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);

    u->OnDelete();
    u->SetDeleted();

    USER_TO_DEL utd;
    utd.iter = u;
    utd.delTime = stgTime;
    usersToDelete.push_back(utd);

    UnSetUserNotifiers(u);
    DelUserFromIndexes(u);

    WriteServLog("%s User \'%s\' deleted.",
             admin.GetLogStr().c_str(), login.c_str());

    }
}
//-----------------------------------------------------------------------------
int USERS::ReadUsers()
{
vector<string> usersList;
usersList.clear();
if (store->GetUsersList(&usersList) < 0)
    {
    WriteServLog(store->GetStrError().c_str());
    exit(1);
    }

user_iter ui;

for (unsigned int i = 0; i < usersList.size(); i++)
    {
    USER u(settings, store, tariffs, sysAdmin, &ipIndex);

    u.SetLogin(usersList[i]);
    users.push_front(u);
    ui = users.begin();

    AddUserIntoIndexes(ui);
    SetUserNotifiers(ui);

    if (ui->ReadConf() < 0)
        return -1;

    if (ui->ReadStat() < 0)
        return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
void * USERS::Run(void * d)
{
printfd(__FILE__, "=====================| pid: %d |===================== \n", getpid());
USERS * us = (USERS*) d;

struct tm * t;
time_t tt = stgTime;
t = localtime(&tt);

int min = t->tm_min;
int day = t->tm_mday;

printfd(__FILE__,"Day = %d Min = %d\n", day, min);

time_t touchTime = stgTime - MONITOR_TIME_DELAY_SEC;
string monFile = us->settings->GetMonitorDir() + "/users_r";
printfd(__FILE__, "Monitor=%d file USERS %s\n", us->settings->GetMonitoring(), monFile.c_str());

us->isRunning = true;
while (us->nonstop)
    {
    //printfd(__FILE__,"New Minute. old = %02d current = %02d\n", min, t->tm_min);
    //printfd(__FILE__,"New Day.    old = %2d current = %2d\n", day, t->tm_mday);

    for_each(us->users.begin(), us->users.end(), mem_fun_ref(&USER::Run));

    tt = stgTime;
    t = localtime(&tt);

    if (min != t->tm_min)
        {
        printfd(__FILE__,"Sec = %d\n", stgTime);
        printfd(__FILE__,"New Minute. old = %d current = %d\n", min, t->tm_min);
        min = t->tm_min;

        us->NewMinute(t);
        }

    t = localtime(&tt);
    if (day != t->tm_mday)
        {
        printfd(__FILE__,"Sec = %d\n", stgTime);
        printfd(__FILE__,"New Day. old = %d current = %d\n", day, t->tm_mday);
        day = t->tm_mday;
        us->NewDay(t);
        }

    if (us->settings->GetMonitoring() && (touchTime + MONITOR_TIME_DELAY_SEC <= stgTime))
        {
        //printfd(__FILE__, "Monitor=%d file TRAFFCOUNTER %s\n", tc->monitoring, monFile.c_str());
        touchTime = stgTime;
        TouchFile(monFile.c_str());
        }

    stgUsleep(100000);
    } //while (us->nonstop)

user_iter ui = us->users.begin();
while (ui != us->users.end())
    {
    us->UnSetUserNotifiers(ui);
    us->DelUserFromIndexes(ui);
    ui++;
    }

list<USER_TO_DEL>::iterator iter;
iter = us->usersToDelete.begin();
while (iter != us->usersToDelete.end())
    {
    iter->delTime -= 2 * userDeleteDelayTime;
    ++iter;
    }
us->RealDelUser();

us->isRunning = false;

return NULL;
}
//-----------------------------------------------------------------------------
void USERS::NewMinute(const struct tm * t)
{
int usersCnt = 0;
list<USER>::iterator usr;

//Write traff, reset session traff. Fake disconnect-connect
if (t->tm_hour == 23 && t->tm_min == 59)
    {
    printfd(__FILE__,"MidnightResetSessionStat\n");
    for_each(users.begin(), users.end(), mem_fun_ref(&USER::MidnightResetSessionStat));
    }

if (TimeToWriteDetailStat(t))
    {
    //printfd(__FILE__, "USER::WriteInetStat\n");
    for_each(users.begin(), users.end(), mem_fun_ref(&USER::SwapDetailStat));
    usersCnt = 0;

    // Пишем юзеров частями. В перерывах вызываем USER::Run
    usr = users.begin();
    while (usr != users.end())
        {
        usersCnt++;
        usr->WriteDetailStat();
        usr++;
        if (usersCnt % 10 == 0)
            for_each(users.begin(), users.end(), mem_fun_ref(&USER::Run));
        }

    for_each(users.begin(), users.end(), mem_fun_ref(&USER::ResetDetailStat));
    }

RealDelUser();
}
//-----------------------------------------------------------------------------
void USERS::NewDay(const struct tm * t)
{
struct tm * t1;
time_t tt = stgTime;
t1 = localtime(&tt);
int dayFee = settings->GetDayFee();

if (dayFee == 0)
    dayFee = DaysInCurrentMonth();

printfd(__FILE__, "DayFee = %d\n", dayFee);
printfd(__FILE__, "Today = %d DayResetTraff = %d\n", t1->tm_mday, settings->GetDayResetTraff());
printfd(__FILE__, "DayFeeIsLastDay = %d\n", settings->GetDayFeeIsLastDay());

if (!settings->GetDayFeeIsLastDay())
    {
    printfd(__FILE__, "DayResetTraff - 1 -\n");
    DayResetTraff(t1);
    //printfd(__FILE__, "DayResetTraff - 1 - 1 -\n");
    }

if (settings->GetSpreadFee())
    {
    printfd(__FILE__, "Spread DayFee\n");
    for_each(users.begin(), users.end(), mem_fun_ref(&USER::ProcessDayFeeSpread));
    }
else
    {
    if (t->tm_mday == dayFee)
        {
        printfd(__FILE__, "DayFee\n");
        for_each(users.begin(), users.end(), mem_fun_ref(&USER::ProcessDayFee));
        }
    }

if (settings->GetDayFeeIsLastDay())
    {
    printfd(__FILE__, "DayResetTraff - 2 -\n");
    DayResetTraff(t1);
    }
}
//-----------------------------------------------------------------------------
void USERS::DayResetTraff(const struct tm * t1)
{
int dayResetTraff = settings->GetDayResetTraff();
if (dayResetTraff == 0)
    dayResetTraff = DaysInCurrentMonth();
if (t1->tm_mday == dayResetTraff)
    {
    printfd(__FILE__, "ResetTraff\n");
    for_each(users.begin(), users.end(), mem_fun_ref(&USER::ProcessNewMonth));
    for_each(users.begin(), users.end(), mem_fun_ref(&USER::SetPrepaidTraff));
    }
}
//-----------------------------------------------------------------------------
int USERS::Start()
{
if (ReadUsers())
    {
    WriteServLog("USERS: Error: Cannot read users!");
    return -1;
    }

nonstop = true;
if (pthread_create(&thread, NULL, Run, this))
    {
    WriteServLog("USERS: Error: Cannot start thread!");
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int USERS::Stop()
{
printfd(__FILE__, "USERS::Stop()\n");

if (!isRunning)
    {
    //printfd(__FILE__, "Alredy stopped\n");
    return 0;
    }

nonstop = false;

//5 seconds to thread stops itself
unsigned i;
for (i = 0; i < 25 * (users.size() / 50 + 1); i++)
    {
    if (!isRunning)
        break;

    usleep(200000);
    }

//after 5 seconds waiting thread still running. now kill it
if (isRunning)
    {
    printfd(__FILE__, "kill USERS thread.\n");
    //TODO pthread_cancel()
    if (pthread_kill(thread, SIGINT))
        {
        //errorStr = "Cannot kill USERS thread.";
        //printfd(__FILE__, "Cannot kill USERS thread.\n");
        //return 0;
        }
    printfd(__FILE__, "USERS killed\n");
    }

printfd(__FILE__, "Before USERS::Run()\n");
for_each(users.begin(), users.end(), mem_fun_ref(&USER::Run));
for_each(users.begin(), users.end(), mem_fun_ref(&USER::SwapDetailStat));
for_each(users.begin(), users.end(), mem_fun_ref(&USER::WriteDetailStat));
for_each(users.begin(), users.end(), mem_fun_ref(&USER::WriteStat));
for_each(users.begin(), users.end(), mem_fun_ref(&USER::WriteConf));

printfd(__FILE__, "USERS::Stop()\n");
return 0;
}
//-----------------------------------------------------------------------------
void USERS::RealDelUser()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

printfd(__FILE__, "RealDelUser() users to del: %d\n", usersToDelete.size());

list<USER_TO_DEL>::iterator iter;
iter = usersToDelete.begin();
while (iter != usersToDelete.end())
    {
    printfd(__FILE__, "RealDelUser() user=%s\n", iter->iter->GetLogin().c_str());
    if (iter->delTime + userDeleteDelayTime < stgTime)
        {
        printfd(__FILE__, "RealDelUser() user=%s removed from DB\n", iter->iter->GetLogin().c_str());
        if (store->DelUser(iter->iter->GetLogin()))
            {
            WriteServLog("Error removing user \'%s\' from database.", iter->iter->GetLogin().c_str());
            }
        users.erase(iter->iter);
        usersToDelete.erase(iter++);
        }
    else
        {
        ++iter;
        }
    }
return;
}
//-----------------------------------------------------------------------------
int USERS::GetUserNum()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
return users.size();
}
//-----------------------------------------------------------------------------
void USERS::AddToIPIdx(user_iter user)
{
printfd(__FILE__, "USERS: Add IP Idx\n");
uint32_t ip = user->GetCurrIP();
//assert(ip && "User has non-null ip");
if (!ip)
    return; // User has disconnected

STG_LOCKER lock(&mutex, __FILE__, __LINE__);

const map<uint32_t, user_iter>::iterator it(
        ipIndex.lower_bound(ip)
);

assert((it == ipIndex.end() || it->first != ip) && "User is not in index");

ipIndex.insert(it, std::make_pair(ip, user));
}
//-----------------------------------------------------------------------------
void USERS::DelFromIPIdx(uint32_t ip)
{
printfd(__FILE__, "USERS: Del IP Idx\n");
assert(ip && "User has non-null ip");

STG_LOCKER lock(&mutex, __FILE__, __LINE__);

const map<uint32_t, user_iter>::iterator it(
        ipIndex.find(ip)
);

//assert(it != ipIndex.end() && "User is in index");
if (it == ipIndex.end())
    return; // User has not been added

ipIndex.erase(it);
}
//-----------------------------------------------------------------------------
int USERS::FindByIPIdx(uint32_t ip, user_iter * usr)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

map<uint32_t, user_iter>::iterator it;
it = ipIndex.find(ip);

if (it == ipIndex.end())
    {
    //printfd(__FILE__, "User NOT found in IP_Index!!!\n");
    return -1;
    }
*usr = it->second;
//printfd(__FILE__, "User found in IP_Index\n");
return 0;
}
//-----------------------------------------------------------------------------
void USERS::AddNotifierUserAdd(NOTIFIER_BASE<user_iter> * n)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
onAddNotifiers.insert(n);
}
//-----------------------------------------------------------------------------
void USERS::DelNotifierUserAdd(NOTIFIER_BASE<user_iter> * n)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
//printfd(__FILE__, "DelNotifierUserAdd\n");
onAddNotifiers.erase(n);
}
//-----------------------------------------------------------------------------
void USERS::AddNotifierUserDel(NOTIFIER_BASE<user_iter> * n)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
onDelNotifiers.insert(n);
}
//-----------------------------------------------------------------------------
void USERS::DelNotifierUserDel(NOTIFIER_BASE<user_iter> * n)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
onDelNotifiers.erase(n);
}
//-----------------------------------------------------------------------------
int USERS::OpenSearch()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
handle++;
searchDescriptors[handle] = users.begin();
return handle;
}
//-----------------------------------------------------------------------------
int USERS::SearchNext(int h, user_iter * u)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (searchDescriptors.find(h) == searchDescriptors.end())
    {
    WriteServLog("USERS. Incorrect search handle.");
    return -1;
    }

if (searchDescriptors[h] == users.end())
    return -1;

while (searchDescriptors[h]->GetDeleted())
    {
    ++searchDescriptors[h];
    if (searchDescriptors[h] == users.end())
        {
        return -1;
        }
    }

*u = searchDescriptors[h];

++searchDescriptors[h];

return 0;
}
//-----------------------------------------------------------------------------
int USERS::CloseSearch(int h)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
if (searchDescriptors.find(h) != searchDescriptors.end())
    {
    searchDescriptors.erase(searchDescriptors.find(h));
    return 0;
    }

WriteServLog("USERS. Incorrect search handle.");
return -1;
}
//-----------------------------------------------------------------------------
void USERS::SetUserNotifiers(user_iter user)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

PROPERTY_NOTIFER_IP_BEFORE nb(*this, user);
PROPERTY_NOTIFER_IP_AFTER  na(*this, user);

userIPNotifiersBefore.push_front(nb);
userIPNotifiersAfter.push_front(na);

user->AddCurrIPBeforeNotifier(&(*userIPNotifiersBefore.begin()));
user->AddCurrIPAfterNotifier(&(*userIPNotifiersAfter.begin()));
}
//-----------------------------------------------------------------------------
void USERS::UnSetUserNotifiers(user_iter user)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

list<PROPERTY_NOTIFER_IP_BEFORE>::iterator  bi;
list<PROPERTY_NOTIFER_IP_AFTER>::iterator   ai;

bi = userIPNotifiersBefore.begin();
while (bi != userIPNotifiersBefore.end())
    {
    if (bi->GetUser() == user)
        {
        bi->GetUser()->DelCurrIPBeforeNotifier(&(*bi));
        userIPNotifiersBefore.erase(bi);
        //printfd(__FILE__, "Notifier Before removed. User %s\n", bi->GetUser()->GetLogin().c_str());
        break;
        }
    bi++;
    }

ai = userIPNotifiersAfter.begin();
while (ai != userIPNotifiersAfter.end())
    {
    if (ai->GetUser() == user)
        {
        ai->GetUser()->DelCurrIPAfterNotifier(&(*ai));
        userIPNotifiersAfter.erase(ai);
        //printfd(__FILE__, "Notifier After removed. User %s\n", ai->GetUser()->GetLogin().c_str());
        break;
        }
    ai++;
    }
}
//-----------------------------------------------------------------------------
void USERS::AddUserIntoIndexes(user_iter user)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
loginIndex.insert(pair<string, user_iter>(user->GetLogin(), user));
}
//-----------------------------------------------------------------------------
void USERS::DelUserFromIndexes(user_iter user)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
loginIndex.erase(user->GetLogin());
}
//-----------------------------------------------------------------------------
bool USERS::TimeToWriteDetailStat(const struct tm * t)
{
int statTime = settings->GetDetailStatWritePeriod();

switch (statTime)
    {
    case dsPeriod_1:
        if (t->tm_min == 0)
            return true;
        break;
    case dsPeriod_1_2:
        if (t->tm_min % 30 == 0)
            return true;
        break;
    case dsPeriod_1_4:
        if (t->tm_min % 15 == 0)
            return true;
        break;
    case dsPeriod_1_6:
        if (t->tm_min % 10 == 0)
            return true;
        break;
    }
return false;
}
//-----------------------------------------------------------------------------
/*int USERS::SendMessage(const string & login,
                       time_t sndTtime,
                       time_t showTime,
                       char type,
                       const string & text) const
{
return 0;
}*/
//-----------------------------------------------------------------------------



