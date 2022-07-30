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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#pragma once

#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/notifer.h"
#include "stg/user.h"
#include "stg/blowfish.h"
#include "stg/rs_packets.h"
#include "stg/logger.h"

#include "nrmap_parser.h"

#include <string>
#include <list>
#include <map>
#include <functional>
#include <utility>
#include <mutex>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop
#include <cstdint>

namespace STG
{
struct Settings;
struct Settings;
}

namespace RS
{

class REMOTE_SCRIPT;
class UpdateRouter;
class DisconnectUser;

using UserPtr = STG::User*;

//-----------------------------------------------------------------------------
class ADD_USER_NONIFIER: public STG::NotifierBase<UserPtr> {
public:
    explicit ADD_USER_NONIFIER(REMOTE_SCRIPT & r)
        : rs(r) {}
    void Notify(const UserPtr & user);

private:
    ADD_USER_NONIFIER(const ADD_USER_NONIFIER & rhs);
    ADD_USER_NONIFIER & operator=(const ADD_USER_NONIFIER);

    REMOTE_SCRIPT & rs;
};
//-----------------------------------------------------------------------------
class DEL_USER_NONIFIER: public STG::NotifierBase<UserPtr> {
public:
    explicit DEL_USER_NONIFIER(REMOTE_SCRIPT & r)
        : rs(r) {}
    void Notify(const UserPtr & user);

private:
    DEL_USER_NONIFIER(const DEL_USER_NONIFIER & rhs);
    DEL_USER_NONIFIER & operator=(const DEL_USER_NONIFIER);

    REMOTE_SCRIPT & rs;
};
//-----------------------------------------------------------------------------
class IP_NOTIFIER: public STG::PropertyNotifierBase<uint32_t> {
public:
    IP_NOTIFIER(REMOTE_SCRIPT & r, UserPtr u)
        : user(u), rs(r) { user->AddCurrIPAfterNotifier(this); }
    IP_NOTIFIER(const IP_NOTIFIER & rhs)
        : user(rhs.user), rs(rhs.rs) { user->AddCurrIPAfterNotifier(this); }
    ~IP_NOTIFIER() { user->DelCurrIPAfterNotifier(this); }

    IP_NOTIFIER & operator=(const IP_NOTIFIER & rhs)
    {
        user->DelCurrIPAfterNotifier(this);
        user = rhs.user;
        user->AddCurrIPAfterNotifier(this);
        return *this;
    }

    void Notify(const uint32_t & oldValue, const uint32_t & newValue);
    UserPtr GetUser() const { return user; }

private:

    UserPtr user;
    REMOTE_SCRIPT & rs;
};
//-----------------------------------------------------------------------------
class CONNECTED_NOTIFIER: public STG::PropertyNotifierBase<bool> {
public:
    CONNECTED_NOTIFIER(REMOTE_SCRIPT & r, UserPtr u)
        : user(u), rs(r) { user->AddConnectedAfterNotifier(this); }
    CONNECTED_NOTIFIER(const CONNECTED_NOTIFIER & rhs)
        : user(rhs.user), rs(rhs.rs) { user->AddConnectedAfterNotifier(this); }
    ~CONNECTED_NOTIFIER() { user->DelConnectedAfterNotifier(this); }

    CONNECTED_NOTIFIER & operator=(const CONNECTED_NOTIFIER & rhs)
    {
        user->DelConnectedAfterNotifier(this);
        user = rhs.user;
        user->AddConnectedAfterNotifier(this);
        return *this;
    }

    void Notify(const bool & oldValue, const bool & newValue);
    UserPtr GetUser() const { return user; }

private:

    UserPtr user;
    REMOTE_SCRIPT & rs;
};
//-----------------------------------------------------------------------------
struct USER {
    USER(const std::vector<uint32_t> & r, UserPtr it)
        : lastSentTime(0),
          user(it),
          routers(r),
          shortPacketsCount(0),
          ip(user->GetCurrIP())
    {}

    time_t lastSentTime;
    UserPtr user;
    std::vector<uint32_t> routers;
    int shortPacketsCount;
    uint32_t ip;
};
//-----------------------------------------------------------------------------
class SETTINGS {
public:
                        SETTINGS();
    virtual             ~SETTINGS() {}
    const std::string & GetStrError() const { return errorStr; }
    int                 ParseSettings(const STG::ModuleSettings & s);
    int                 GetSendPeriod() const { return sendPeriod; }
    uint16_t            GetPort() const { return port; }
    const std::vector<NET_ROUTER> & GetSubnetsMap() const { return netRouters; }
    const std::vector<std::string> & GetUserParams() const { return userParams; }
    const std::string & GetPassword() const { return password; }
    const std::string & GetMapFileName() const { return subnetFile; }

private:
    int                 sendPeriod;
    uint16_t            port;
    std::string         errorStr;
    std::vector<NET_ROUTER> netRouters;
    std::vector<std::string> userParams;
    std::string         password;
    std::string         subnetFile;
};
//-----------------------------------------------------------------------------
class REMOTE_SCRIPT : public STG::Plugin {
public:
                        REMOTE_SCRIPT();

    void                SetUsers(STG::Users * u) override { users = u; }
    void                SetSettings(const STG::ModuleSettings & s) override { settings = s; }
    int                 ParseSettings() override;

    int                 Start() override;
    int                 Stop() override;
    int                 Reload(const STG::ModuleSettings & ms) override;
    bool                IsRunning() override { return isRunning; }

    const std::string & GetStrError() const override { return errorStr; }
    std::string         GetVersion() const override { return "Remote script v 0.3"; }
    uint16_t            GetStartPosition() const override { return 10; }
    uint16_t            GetStopPosition() const override { return 10; }

    void                DelUser(UserPtr u) { UnSetUserNotifiers(u); }
    void                AddUser(UserPtr u) { SetUserNotifiers(u); }

    void                AddRSU(UserPtr user);
    void                DelRSU(UserPtr user);

private:
    REMOTE_SCRIPT(const REMOTE_SCRIPT & rhs);
    REMOTE_SCRIPT & operator=(const REMOTE_SCRIPT & rhs);

    void                Run(std::stop_token token);
    bool                PrepareNet();
    bool                FinalizeNet();

    bool                Send(USER & rsu, bool forceDisconnect = false) const;
    bool                SendDirect(USER & rsu, uint32_t routerIP, bool forceDisconnect = false) const;
    bool                PreparePacket(char * buf, size_t bufSize, USER &rsu, bool forceDisconnect = false) const;
    void                PeriodicSend();

    std::vector<uint32_t> IP2Routers(uint32_t ip);
    bool                GetUsers();

    void                SetUserNotifiers(UserPtr u);
    void                UnSetUserNotifiers(UserPtr u);

    void                InitEncrypt(const std::string & password) const;
    void                Encrypt(void * dst, const void * src, size_t len8) const;

    mutable BLOWFISH_CTX ctx;

    std::list<IP_NOTIFIER> ipNotifierList;
    std::list<CONNECTED_NOTIFIER> connNotifierList;
    std::map<uint32_t, USER> authorizedUsers;

    mutable std::string errorStr;
    SETTINGS         rsSettings;
    STG::ModuleSettings     settings;
    int                 sendPeriod;
    int                 halfPeriod;

    bool                isRunning;

    STG::Users *             users;

    std::vector<NET_ROUTER> netRouters;

    std::jthread        m_thread;
    std::mutex          m_mutex;

    int                 sock;

    ADD_USER_NONIFIER onAddUserNotifier;
    DEL_USER_NONIFIER onDelUserNotifier;

    STG::PluginLogger       logger;

    friend class RS::UpdateRouter;
    friend class RS::DisconnectUser;
    friend class RS::CONNECTED_NOTIFIER;
};
//-----------------------------------------------------------------------------
class DisconnectUser : public std::unary_function<std::pair<const uint32_t, USER> &, void> {
    public:
        explicit DisconnectUser(REMOTE_SCRIPT & rs) : rscript(rs) {}
        void operator()(std::pair<const uint32_t, USER> & p)
        {
            rscript.Send(p.second, true);
        }
    private:
        REMOTE_SCRIPT & rscript;
};
//-----------------------------------------------------------------------------
inline void ADD_USER_NONIFIER::Notify(const UserPtr & user)
{
rs.AddUser(user);
}
//-----------------------------------------------------------------------------
inline void DEL_USER_NONIFIER::Notify(const UserPtr & user)
{
rs.DelUser(user);
}
//-----------------------------------------------------------------------------

} // namespace RS
