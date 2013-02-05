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
$Revision: 1.30 $
$Date: 2010/03/04 12:29:06 $
$Author: faust $
*/

#include <unistd.h>

#include <csignal>
#include <cassert>
#include <algorithm> // for_each
#include <functional> // mem_fun_ref

#include "stg/user.h"
#include "stg/users.h"
#include "stg/user_property.h"
#include "stg/common.h"
#include "stg/plugin_creator.h"
#include "ao.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static PLUGIN_CREATOR<AUTH_AO> aoc;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN * GetPlugin()
{
return aoc.GetPlugin();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename varType>
class IS_CONTAINS_USER: public std::binary_function<varType, USER_PTR, bool>
{
public:
    bool operator()(varType notifier, USER_PTR user) const
        {
        return notifier.GetUser() == user;
        }
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
std::string AUTH_AO::GetVersion() const
{
return "Always Online authorizator v.1.0";
}
//-----------------------------------------------------------------------------
AUTH_AO::AUTH_AO()
    : errorStr(),
      users(NULL),
      usersList(),
      isRunning(false),
      settings(),
      BeforeChgAONotifierList(),
      AfterChgAONotifierList(),
      BeforeChgIPNotifierList(),
      AfterChgIPNotifierList(),
      onAddUserNotifier(*this),
      onDelUserNotifier(*this),
      logger(GetPluginLogger(GetStgLogger(), "auth_ao"))
{
}
//-----------------------------------------------------------------------------
int AUTH_AO::Start()
{
printfd(__FILE__, "AUTH_AO::Start()\n");
GetUsers();

users->AddNotifierUserAdd(&onAddUserNotifier);
users->AddNotifierUserDel(&onDelUserNotifier);

std::for_each(usersList.begin(), usersList.end(), std::bind1st(std::mem_fun(&AUTH_AO::UpdateUserAuthorization), this));

isRunning = true;

return 0;
}
//-----------------------------------------------------------------------------
int AUTH_AO::Stop()
{
printfd(__FILE__, "AUTH_AO::Stop()\n");
if (!isRunning)
    return 0;

users->DelNotifierUserAdd(&onAddUserNotifier);
users->DelNotifierUserDel(&onDelUserNotifier);

std::list<USER_PTR>::iterator users_iter;
users_iter = usersList.begin();
while (users_iter != usersList.end())
    {
    if ((*users_iter)->IsAuthorizedBy(this))
        users->Unauthorize((*users_iter)->GetLogin(), this);
    UnSetUserNotifiers(*users_iter);
    ++users_iter;
    }
isRunning = false;
return 0;
}
//-----------------------------------------------------------------------------
void AUTH_AO::SetUserNotifiers(USER_PTR u)
{
// ---------- AlwaysOnline -------------------
CHG_BEFORE_NOTIFIER<int> BeforeChgAONotifier(*this, u);
CHG_AFTER_NOTIFIER<int>  AfterChgAONotifier(*this, u);

BeforeChgAONotifierList.push_front(BeforeChgAONotifier);
AfterChgAONotifierList.push_front(AfterChgAONotifier);

u->GetProperty().alwaysOnline.AddBeforeNotifier(&BeforeChgAONotifierList.front());
u->GetProperty().alwaysOnline.AddAfterNotifier(&AfterChgAONotifierList.front());
// ---------- AlwaysOnline end ---------------

// ---------- IP -------------------
CHG_BEFORE_NOTIFIER<USER_IPS> BeforeChgIPNotifier(*this, u);
CHG_AFTER_NOTIFIER<USER_IPS>  AfterChgIPNotifier(*this, u);

BeforeChgIPNotifierList.push_front(BeforeChgIPNotifier);
AfterChgIPNotifierList.push_front(AfterChgIPNotifier);

u->GetProperty().ips.AddBeforeNotifier(&BeforeChgIPNotifierList.front());
u->GetProperty().ips.AddAfterNotifier(&AfterChgIPNotifierList.front());
// ---------- IP end ---------------
}
//-----------------------------------------------------------------------------
void AUTH_AO::UnSetUserNotifiers(USER_PTR u)
{
// ---      AlwaysOnline        ---
IS_CONTAINS_USER<CHG_BEFORE_NOTIFIER<int> > IsContainsUserAOB;
IS_CONTAINS_USER<CHG_AFTER_NOTIFIER<int> > IsContainsUserAOA;

std::list<CHG_BEFORE_NOTIFIER<int> >::iterator aoBIter;
std::list<CHG_AFTER_NOTIFIER<int> >::iterator  aoAIter;

aoBIter = find_if(BeforeChgAONotifierList.begin(),
                  BeforeChgAONotifierList.end(),
                  bind2nd(IsContainsUserAOB, u));

if (aoBIter != BeforeChgAONotifierList.end())
    {
    aoBIter->GetUser()->GetProperty().alwaysOnline.DelBeforeNotifier(&(*aoBIter));
    BeforeChgAONotifierList.erase(aoBIter);
    }

aoAIter = find_if(AfterChgAONotifierList.begin(),
                  AfterChgAONotifierList.end(),
                  bind2nd(IsContainsUserAOA, u));

if (aoAIter != AfterChgAONotifierList.end())
    {
    aoAIter->GetUser()->GetProperty().alwaysOnline.DelAfterNotifier(&(*aoAIter));
    AfterChgAONotifierList.erase(aoAIter);
    }
// ---      AlwaysOnline end    ---

// ---          IP              ---
IS_CONTAINS_USER<CHG_BEFORE_NOTIFIER<USER_IPS> > IsContainsUserIPB;
IS_CONTAINS_USER<CHG_AFTER_NOTIFIER<USER_IPS> >  IsContainsUserIPA;

std::list<CHG_BEFORE_NOTIFIER<USER_IPS> >::iterator ipBIter;
std::list<CHG_AFTER_NOTIFIER<USER_IPS> >::iterator  ipAIter;

ipBIter = std::find_if(BeforeChgIPNotifierList.begin(),
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
void AUTH_AO::GetUsers()
{
USER_PTR u;
int h = users->OpenSearch();
assert(h && "USERS::OpenSearch is always correct");

while (!users->SearchNext(h, &u))
    {
    usersList.push_back(u);
    SetUserNotifiers(u);
    }

users->CloseSearch(h);
}
//-----------------------------------------------------------------------------
void AUTH_AO::UpdateUserAuthorization(CONST_USER_PTR u) const
{
if (u->GetProperty().alwaysOnline)
    {
    USER_IPS ips = u->GetProperty().ips;
    if (ips.OnlyOneIP())
        {
        users->Authorize(u->GetLogin(), ips[0].ip, 0xFFffFFff, this);
        }
    }
}
//-----------------------------------------------------------------------------
void AUTH_AO::AddUser(USER_PTR u)
{
SetUserNotifiers(u);
usersList.push_back(u);
UpdateUserAuthorization(u);
}
//-----------------------------------------------------------------------------
void AUTH_AO::DelUser(USER_PTR u)
{
users->Unauthorize(u->GetLogin(), this);
UnSetUserNotifiers(u);
usersList.remove(u);
}
//-----------------------------------------------------------------------------
int AUTH_AO::SendMessage(const STG_MSG &, uint32_t) const
{
errorStr = "Authorization modele \'AlwaysOnline\' does not support sending messages";
return -1;
}
//-----------------------------------------------------------------------------
template <typename varParamType>
void CHG_BEFORE_NOTIFIER<varParamType>::Notify(const varParamType &, const varParamType &)
{
//EVENT_LOOP_SINGLETON::GetInstance().Enqueue(auth, &AUTH_AO::Unauthorize, user);
if (user->IsAuthorizedBy(&auth))
    auth.users->Unauthorize(user->GetLogin(), &auth);
}
//-----------------------------------------------------------------------------
template <typename varParamType>
void CHG_AFTER_NOTIFIER<varParamType>::Notify(const varParamType &, const varParamType &)
{
//EVENT_LOOP_SINGLETON::GetInstance().Enqueue(auth, &AUTH_AO::UpdateUserAuthorization, user);
auth.UpdateUserAuthorization(user);
}
//-----------------------------------------------------------------------------
