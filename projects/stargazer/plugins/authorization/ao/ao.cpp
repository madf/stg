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

using STG::AUTH_AO;

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
      logger(PluginLogger::get("auth_ao"))
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

m_conns.clear();

isRunning = false;
return 0;
}
//-----------------------------------------------------------------------------
void AUTH_AO::SetUserNotifiers(UserPtr u)
{
    m_conns.emplace_back(
        u->GetID(),
        u->GetProperties().alwaysOnline.beforeChange([this, u](auto, auto){ Unauthorize(u); }),
        u->GetProperties().alwaysOnline.afterChange([this, u](auto, auto){ UpdateUserAuthorization(u); }),
        u->GetProperties().ips.beforeChange([this, u](const auto&, const auto&){ Unauthorize(u); }),
        u->GetProperties().ips.afterChange([this, u](const auto&, const auto&){ UpdateUserAuthorization(u); })
    );
}
//-----------------------------------------------------------------------------
void AUTH_AO::UnSetUserNotifiers(UserPtr u)
{
    m_conns.erase(std::remove_if(m_conns.begin(), m_conns.end(),
                                 [u](const auto& c){ return std::get<0>(c) == u->GetID(); }),
                  m_conns.end());
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
int AUTH_AO::SendMessage(const Message &, uint32_t) const
{
errorStr = "Authorization modele \'AlwaysOnline\' does not support sending messages";
return -1;
}
//-----------------------------------------------------------------------------
void AUTH_AO::Unauthorize(UserPtr user)
{
    if (user->IsAuthorizedBy(this))
        users->Unauthorize(user->GetLogin(), this);
}
