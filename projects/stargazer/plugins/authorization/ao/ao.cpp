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

#include "ao.h"

#include "stg/user.h"
#include "stg/users.h"
#include "stg/user_property.h"
#include "stg/common.h"

#include <algorithm> // for_each
#include <functional> // mem_fun_ref
#include <csignal>
#include <cassert>

#include <unistd.h>

extern "C" STG::Plugin* GetPlugin()
{
    static AUTH_AO plugin;
    return &plugin;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
std::string AUTH_AO::GetVersion() const
{
return "Always Online authorizator v.1.0";
}
//-----------------------------------------------------------------------------
AUTH_AO::AUTH_AO()
    : users(NULL),
      isRunning(false),
      logger(STG::PluginLogger::get("auth_ao"))
{
}
//-----------------------------------------------------------------------------
int AUTH_AO::Start()
{
printfd(__FILE__, "AUTH_AO::Start()\n");
GetUsers();

m_onAddUserConn = users->onAdd([this](auto user){ AddUser(user); });
m_onDelUserConn = users->onDel([this](auto user){ DelUser(user); });

std::for_each(userList.begin(), userList.end(), [this](auto user){ UpdateUserAuthorization(user); });

isRunning = true;

return 0;
}
//-----------------------------------------------------------------------------
int AUTH_AO::Stop()
{
printfd(__FILE__, "AUTH_AO::Stop()\n");
if (!isRunning)
    return 0;

m_onAddUserConn.disconnect();
m_onDelUserConn.disconnect();

auto it = userList.begin();
while (it != userList.end())
    {
    if ((*it)->IsAuthorizedBy(this))
        users->Unauthorize((*it)->GetLogin(), this);
    UnSetUserNotifiers(*it);
    ++it;
    }
isRunning = false;
return 0;
}
//-----------------------------------------------------------------------------
void AUTH_AO::SetUserNotifiers(UserPtr u)
{
// ---------- AlwaysOnline -------------------
CHG_BEFORE_NOTIFIER<int> BeforeChgAONotifier(*this, u);
CHG_AFTER_NOTIFIER<int>  AfterChgAONotifier(*this, u);

BeforeChgAONotifierList.push_front(BeforeChgAONotifier);
AfterChgAONotifierList.push_front(AfterChgAONotifier);

u->GetProperties().alwaysOnline.AddBeforeNotifier(&BeforeChgAONotifierList.front());
u->GetProperties().alwaysOnline.AddAfterNotifier(&AfterChgAONotifierList.front());
// ---------- AlwaysOnline end ---------------

// ---------- IP -------------------
CHG_BEFORE_NOTIFIER<STG::UserIPs> BeforeChgIPNotifier(*this, u);
CHG_AFTER_NOTIFIER<STG::UserIPs>  AfterChgIPNotifier(*this, u);

BeforeChgIPNotifierList.push_front(BeforeChgIPNotifier);
AfterChgIPNotifierList.push_front(AfterChgIPNotifier);

u->GetProperties().ips.AddBeforeNotifier(&BeforeChgIPNotifierList.front());
u->GetProperties().ips.AddAfterNotifier(&AfterChgIPNotifierList.front());
// ---------- IP end ---------------
}
//-----------------------------------------------------------------------------
void AUTH_AO::UnSetUserNotifiers(UserPtr u)
{
// ---      AlwaysOnline        ---
auto aoBIter = find_if(BeforeChgAONotifierList.begin(),
                       BeforeChgAONotifierList.end(),
                       [u](auto notifier){ return notifier.GetUser() == u; });

if (aoBIter != BeforeChgAONotifierList.end())
    {
    aoBIter->GetUser()->GetProperties().alwaysOnline.DelBeforeNotifier(&(*aoBIter));
    BeforeChgAONotifierList.erase(aoBIter);
    }

auto aoAIter = find_if(AfterChgAONotifierList.begin(),
                       AfterChgAONotifierList.end(),
                       [u](auto notifier){ return notifier.GetUser() == u; });

if (aoAIter != AfterChgAONotifierList.end())
    {
    aoAIter->GetUser()->GetProperties().alwaysOnline.DelAfterNotifier(&(*aoAIter));
    AfterChgAONotifierList.erase(aoAIter);
    }
// ---      AlwaysOnline end    ---

// ---          IP              ---
auto ipBIter = std::find_if(BeforeChgIPNotifierList.begin(),
                            BeforeChgIPNotifierList.end(),
                            [u](auto notifier){ return notifier.GetUser() == u; });

if (ipBIter != BeforeChgIPNotifierList.end())
    {
    ipBIter->GetUser()->GetProperties().ips.DelBeforeNotifier(&(*ipBIter));
    BeforeChgIPNotifierList.erase(ipBIter);
    }

auto ipAIter = find_if(AfterChgIPNotifierList.begin(),
                       AfterChgIPNotifierList.end(),
                       [u](auto notifier){ return notifier.GetUser() == u; });

if (ipAIter != AfterChgIPNotifierList.end())
    {
    ipAIter->GetUser()->GetProperties().ips.DelAfterNotifier(&(*ipAIter));
    AfterChgIPNotifierList.erase(ipAIter);
    }
// ---          IP end          ---
}
//-----------------------------------------------------------------------------
void AUTH_AO::GetUsers()
{
UserPtr u;
int h = users->OpenSearch();
assert(h && "USERS::OpenSearch is always correct");

while (!users->SearchNext(h, &u))
    {
    userList.push_back(u);
    SetUserNotifiers(u);
    }

users->CloseSearch(h);
}
//-----------------------------------------------------------------------------
void AUTH_AO::UpdateUserAuthorization(ConstUserPtr u) const
{
if (u->GetProperties().alwaysOnline)
    {
    auto ips = u->GetProperties().ips.get();
    if (ips.onlyOneIP())
        {
        users->Authorize(u->GetLogin(), ips[0].ip, 0xFFffFFff, this);
        }
    }
}
//-----------------------------------------------------------------------------
void AUTH_AO::AddUser(UserPtr u)
{
SetUserNotifiers(u);
userList.push_back(u);
UpdateUserAuthorization(u);
}
//-----------------------------------------------------------------------------
void AUTH_AO::DelUser(UserPtr u)
{
if (u->IsAuthorizedBy(this))
    users->Unauthorize(u->GetLogin(), this);
UnSetUserNotifiers(u);
userList.erase(std::remove(userList.begin(), userList.end(), u), userList.end());
}
//-----------------------------------------------------------------------------
int AUTH_AO::SendMessage(const STG::Message &, uint32_t) const
{
errorStr = "Authorization modele \'AlwaysOnline\' does not support sending messages";
return -1;
}
//-----------------------------------------------------------------------------
template <typename varParamType>
void CHG_BEFORE_NOTIFIER<varParamType>::notify(const varParamType &, const varParamType &)
{
if (user->IsAuthorizedBy(&auth))
    auth.users->Unauthorize(user->GetLogin(), &auth);
}
//-----------------------------------------------------------------------------
template <typename varParamType>
void CHG_AFTER_NOTIFIER<varParamType>::notify(const varParamType &, const varParamType &)
{
auth.UpdateUserAuthorization(user);
}
//-----------------------------------------------------------------------------
