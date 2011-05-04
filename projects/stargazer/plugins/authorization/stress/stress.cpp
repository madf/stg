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
 $Revision: 1.5 $
 $Date: 2009/06/19 12:50:32 $
 $Author: faust $
 */

#include <stdio.h>
#include <unistd.h>

#include <csignal>
#include <algorithm>

#include "stg/user.h"
#include "stg/common.h"
#include "stg/user_property.h"

#include "stress.h"

class STRESS_CREATOR
{
private:
    AUTH_STRESS * dc;

public:
    STRESS_CREATOR()
        {
        printfd(__FILE__, "constructor STRESS_CREATOR\n");
        dc = new AUTH_STRESS();
        };
    ~STRESS_CREATOR()
        {
        printfd(__FILE__, "destructor STRESS_CREATOR\n");
        delete dc;
        };

    PLUGIN * GetPlugin()
    {
        return dc;
    };
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
STRESS_CREATOR stressc;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Класс для поиска юзера в списке нотификаторов
template <typename varType>
class IS_CONTAINS_USER: public binary_function<varType, USER_PTR, bool>
{
public:
    bool operator()(varType notifier, USER_PTR user) const
        {
        return notifier.GetUser() == user;
        };
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN * GetPlugin()
{
//printf("BASE_CAPTURER * GetCapturer()\n");
return stressc.GetPlugin();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
AUTH_STRESS_SETTINGS::AUTH_STRESS_SETTINGS()
    : averageOnlineTime(0)
{
}
//-----------------------------------------------------------------------------
int AUTH_STRESS_SETTINGS::ParseIntInRange(const string & str, int min, int max, int * val)
{
if (str2x(str.c_str(), *val))
    {
    errorStr = "Incorrect value \'" + str + "\'.";
    return -1;
    }
if (*val < min || *val > max)
    {
    errorStr = "Value \'" + str + "\' out of range.";
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int AUTH_STRESS_SETTINGS::ParseSettings(const MODULE_SETTINGS & s)
{
PARAM_VALUE pv;
vector<PARAM_VALUE>::const_iterator pvi;

pv.param = "AverageOnlineTime";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end())
    {
    errorStr = "Parameter \'" + pv.param + "\' not found.";
    return -1;
    }

if (ParseIntInRange(pvi->value[0], 5, 10*3600, &averageOnlineTime))
    {
    errorStr = "Cannot parse parameter \'" + pv.param + "\': " + errorStr;
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int AUTH_STRESS_SETTINGS::GetAverageOnlineTime() const
{
return averageOnlineTime;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const string AUTH_STRESS::GetVersion() const
{
return "Stress authorizator v.0.1";
}
//-----------------------------------------------------------------------------
AUTH_STRESS::AUTH_STRESS()
{
pthread_mutex_init(&mutex, NULL);
isRunning = false;
}
//-----------------------------------------------------------------------------
void AUTH_STRESS::SetUsers(USERS * u)
{
users = u;
}
//-----------------------------------------------------------------------------
void AUTH_STRESS::SetSettings(const MODULE_SETTINGS & s)
{
settings = s;
}
//-----------------------------------------------------------------------------
int AUTH_STRESS::ParseSettings()
{
int ret = stressSettings.ParseSettings(settings);
if (ret)
    errorStr = stressSettings.GetStrError();
return ret;
}
//-----------------------------------------------------------------------------
const string & AUTH_STRESS::GetStrError() const
{
return errorStr;
}
//-----------------------------------------------------------------------------
int AUTH_STRESS::Start()
{
GetUsers();
nonstop = true;

list<USER_PTR>::iterator users_iter;

onAddUserNotifier.SetAuthorizator(this);
onDelUserNotifier.SetAuthorizator(this);
users->AddNotifierUserAdd(&onAddUserNotifier);
users->AddNotifierUserDel(&onDelUserNotifier);

if (!isRunning)
    {
    if (pthread_create(&thread, NULL, Run, this))
        {
        errorStr = "Cannot create thread.";
        return -1;
        }
    }

users_iter = usersList.begin();
while (users_iter != usersList.end())
    {
    Authorize(*users_iter);
    users_iter++;
    }

//isRunning = true;
return 0;
}
//-----------------------------------------------------------------------------
int AUTH_STRESS::Stop()
{
nonstop = false;
if (isRunning)
    {
    //5 seconds to thread stops itself
    int i;
    for (i = 0; i < 25; i++)
        {
        if (!isRunning)
            break;
        usleep(200000);
        }

    //after 5 seconds waiting thread still running. now killing it
    if (isRunning)
        {
        if (pthread_kill(thread, SIGINT))
            {
            errorStr = "Cannot kill thread.";
            return -1;
            }
        printfd(__FILE__, "AUTH_STRESS killed Run\n");
        }
    }

users->DelNotifierUserAdd(&onAddUserNotifier);
users->DelNotifierUserDel(&onDelUserNotifier);

return 0;
}
//-----------------------------------------------------------------------------
bool AUTH_STRESS::IsRunning()
{
return isRunning;
}
//-----------------------------------------------------------------------------
uint16_t AUTH_STRESS::GetStartPosition() const
{
return 70;
}
//-----------------------------------------------------------------------------
uint16_t AUTH_STRESS::GetStopPosition() const
{
return 70;
}
//-----------------------------------------------------------------------------
void AUTH_STRESS::SetUserNotifiers(USER_PTR u)
{
// ---------- IP -------------------
CHG_BEFORE_NOTIFIER<USER_IPS> BeforeChgIPNotifier;
CHG_AFTER_NOTIFIER<USER_IPS>  AfterChgIPNotifier;

BeforeChgIPNotifier.SetAuthorizator(this);
BeforeChgIPNotifier.SetUser(u);
BeforeChgIPNotifierList.push_front(BeforeChgIPNotifier);

AfterChgIPNotifier.SetAuthorizator(this);
AfterChgIPNotifier.SetUser(u);
AfterChgIPNotifierList.push_front(AfterChgIPNotifier);

u->GetProperty().ips.AddBeforeNotifier(&(*BeforeChgIPNotifierList.begin()));
u->GetProperty().ips.AddAfterNotifier(&(*AfterChgIPNotifierList.begin()));
// ---------- IP end ---------------
}
//-----------------------------------------------------------------------------
void AUTH_STRESS::UnSetUserNotifiers(USER_PTR u)
{
// ---          IP              ---
IS_CONTAINS_USER<CHG_BEFORE_NOTIFIER<USER_IPS> > IsContainsUserIPB;
IS_CONTAINS_USER<CHG_AFTER_NOTIFIER<USER_IPS> >  IsContainsUserIPA;

list<CHG_BEFORE_NOTIFIER<USER_IPS> >::iterator ipBIter;
list<CHG_AFTER_NOTIFIER<USER_IPS> >::iterator  ipAIter;

ipBIter = find_if(BeforeChgIPNotifierList.begin(),
                  BeforeChgIPNotifierList.end(),
                  bind2nd(IsContainsUserIPB, u));

if (ipBIter != BeforeChgIPNotifierList.end())
    {
    ipBIter->GetUser()->GetProperty().ips.DelBeforeNotifier(&(*ipBIter));
    BeforeChgIPNotifierList.erase(ipBIter);
    }

ipAIter = find_if(AfterChgIPNotifierList.begin(),
                  AfterChgIPNotifierList.end(),
                  bind2nd(IsContainsUserIPA, u));

if (ipAIter != AfterChgIPNotifierList.end())
    {
    ipAIter->GetUser()->GetProperty().ips.DelAfterNotifier(&(*ipAIter));
    AfterChgIPNotifierList.erase(ipAIter);
    }
// ---          IP end          ---
}
//-----------------------------------------------------------------------------
void AUTH_STRESS::GetUsers()
{
USER_PTR u;
printfd(__FILE__, "users->OpenSearch() usernum=%d\n", users->GetUserNum());
int h = users->OpenSearch();
if (!h)
    {
    printfd(__FILE__, "users->OpenSearch() error\n");
    return;
    }

while (1)
    {
    if (users->SearchNext(h, &u))
        {
        break;
        }
    usersList.push_back(u);
    SetUserNotifiers(u);
    }

users->CloseSearch(h);
}
//-----------------------------------------------------------------------------
void AUTH_STRESS::Unauthorize(USER_PTR u) const
{
if (!u->IsAuthorizedBy(this))
    return;

printfd(__FILE__, "Unauthorized user %s\n", u->GetLogin().c_str());
u->Unauthorize(this);
}
//-----------------------------------------------------------------------------
void AUTH_STRESS::Authorize(USER_PTR u) const
{
USER_IPS ips = u->GetProperty().ips;
if (ips.OnlyOneIP() && !u->IsAuthorizedBy(this))
    {
    if (u->Authorize(ips[0].ip, 0xFFffFFff, this) == 0)
        {
        printfd(__FILE__, "Authorized user %s\n", u->GetLogin().c_str());
        }
    }
}
//-----------------------------------------------------------------------------
void AUTH_STRESS::AddUser(USER_PTR u)
{
//printfd(__FILE__, "User added to list %s\n", u->GetLogin().c_str());
SetUserNotifiers(u);
usersList.push_back(u);
}
//-----------------------------------------------------------------------------
void AUTH_STRESS::DelUser(USER_PTR u)
{
Unauthorize(u);
UnSetUserNotifiers(u);

list<USER_PTR>::iterator users_iter;
users_iter = usersList.begin();

while (users_iter != usersList.end())
    {
    if (u == *users_iter)
        {
        usersList.erase(users_iter);
        printfd(__FILE__, "User removed from list %s\n", u->GetLogin().c_str());
        break;
        }
    users_iter++;
    }
}
//-----------------------------------------------------------------------------
int AUTH_STRESS::SendMessage(const STG_MSG &, uint32_t) const
{
errorStr = "Authorization modele \'AUTH_STRESS\' does not support sending messages";
return -1;
}
//-----------------------------------------------------------------------------
void * AUTH_STRESS::Run(void * d)
{
AUTH_STRESS * ia;
ia = (AUTH_STRESS *)d;

ia->isRunning = true;

while (ia->nonstop)
    {
    printfd(__FILE__, "AUTH_STRESS::Run - averageTime: %d\n", random() % (2*ia->stressSettings.GetAverageOnlineTime()));

    list<USER_PTR>::iterator users_iter;
    users_iter = ia->usersList.begin();
    while (users_iter != ia->usersList.end())
        {
        if (random() % (2*ia->stressSettings.GetAverageOnlineTime()) == 1)
            {
            ia->Authorize(*users_iter);
            printfd(__FILE__, "AUTH_STRESS::Authorize - user: '%s'\n", (*users_iter)->GetLogin().c_str());
            }
        if (random() % (2*ia->stressSettings.GetAverageOnlineTime()) == 2)
            {
            ia->Unauthorize(*users_iter);
            printfd(__FILE__, "AUTH_STRESS::Unauthorize - user: '%s'\n", (*users_iter)->GetLogin().c_str());
            }

        users_iter++;
        }

    sleep(1);
    }

ia->isRunning = false;
return NULL;
}
//-----------------------------------------------------------------------------
template <typename varParamType>
void CHG_BEFORE_NOTIFIER<varParamType>::Notify(const varParamType &, const varParamType &)
{
auth->Unauthorize(user);
}
//-----------------------------------------------------------------------------
template <typename varParamType>
void CHG_AFTER_NOTIFIER<varParamType>::Notify(const varParamType &, const varParamType &)
{
auth->Authorize(user);
}
//-----------------------------------------------------------------------------
