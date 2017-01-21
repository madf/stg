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

#ifndef RSCRIPT_H
#define RSCRIPT_H

#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/os_int.h"
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

#include <pthread.h>

extern "C" PLUGIN * GetPlugin();

#define RS_DEBUG (1)

#define MAX_SHORT_PCKT  (3)

class SETTINGS;
class USERS;

namespace RS
{

class REMOTE_SCRIPT;
class UpdateRouter;
class DisconnectUser;

//-----------------------------------------------------------------------------
class ADD_USER_NONIFIER: public NOTIFIER_BASE<USER_PTR> {
public:
    explicit ADD_USER_NONIFIER(REMOTE_SCRIPT & r)
        : NOTIFIER_BASE<USER_PTR>(), rs(r) {}
    virtual ~ADD_USER_NONIFIER() {}
    void Notify(const USER_PTR & user);

private:
    ADD_USER_NONIFIER(const ADD_USER_NONIFIER & rhs);
    ADD_USER_NONIFIER & operator=(const ADD_USER_NONIFIER);

    REMOTE_SCRIPT & rs;
};
//-----------------------------------------------------------------------------
class DEL_USER_NONIFIER: public NOTIFIER_BASE<USER_PTR> {
public:
    explicit DEL_USER_NONIFIER(REMOTE_SCRIPT & r)
        : NOTIFIER_BASE<USER_PTR>(), rs(r) {}
    virtual ~DEL_USER_NONIFIER() {}
    void Notify(const USER_PTR & user);

private:
    DEL_USER_NONIFIER(const DEL_USER_NONIFIER & rhs);
    DEL_USER_NONIFIER & operator=(const DEL_USER_NONIFIER);

    REMOTE_SCRIPT & rs;
};
//-----------------------------------------------------------------------------
class IP_NOTIFIER: public PROPERTY_NOTIFIER_BASE<uint32_t> {
public:
    IP_NOTIFIER(REMOTE_SCRIPT & r, USER_PTR u)
        : PROPERTY_NOTIFIER_BASE<uint32_t>(), user(u), rs(r) { user->AddCurrIPAfterNotifier(this); }
    IP_NOTIFIER(const IP_NOTIFIER & rhs)
        : PROPERTY_NOTIFIER_BASE<uint32_t>(), user(rhs.user), rs(rhs.rs) { user->AddCurrIPAfterNotifier(this); }
    ~IP_NOTIFIER() { user->DelCurrIPAfterNotifier(this); }

    IP_NOTIFIER & operator=(const IP_NOTIFIER & rhs)
    {
        user->DelCurrIPAfterNotifier(this);
        user = rhs.user;
        user->AddCurrIPAfterNotifier(this);
        return *this;
    }

    void Notify(const uint32_t & oldValue, const uint32_t & newValue);
    USER_PTR GetUser() const { return user; }

private:

    USER_PTR user;
    REMOTE_SCRIPT & rs;
};
//-----------------------------------------------------------------------------
class CONNECTED_NOTIFIER: public PROPERTY_NOTIFIER_BASE<bool> {
public:
    CONNECTED_NOTIFIER(REMOTE_SCRIPT & r, USER_PTR u)
        : PROPERTY_NOTIFIER_BASE<bool>(), user(u), rs(r) { user->AddConnectedAfterNotifier(this); }
    CONNECTED_NOTIFIER(const CONNECTED_NOTIFIER & rhs)
        : PROPERTY_NOTIFIER_BASE<bool>(), user(rhs.user), rs(rhs.rs) { user->AddConnectedAfterNotifier(this); }
    ~CONNECTED_NOTIFIER() { user->DelConnectedAfterNotifier(this); }

    CONNECTED_NOTIFIER & operator=(const CONNECTED_NOTIFIER & rhs)
    {
        user->DelConnectedAfterNotifier(this);
        user = rhs.user;
        user->AddConnectedAfterNotifier(this);
        return *this;
    }

    void Notify(const bool & oldValue, const bool & newValue);
    USER_PTR GetUser() const { return user; }

private:

    USER_PTR user;
    REMOTE_SCRIPT & rs;
};
//-----------------------------------------------------------------------------
struct USER {
    USER(const std::vector<uint32_t> & r, USER_PTR it)
        : lastSentTime(0),
          user(it),
          routers(r),
          shortPacketsCount(0),
          ip(user->GetCurrIP())
    {}

    time_t lastSentTime;
    USER_PTR user;
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
    int                 ParseSettings(const MODULE_SETTINGS & s);
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
class REMOTE_SCRIPT : public PLUGIN {
public:
                        REMOTE_SCRIPT();
    virtual             ~REMOTE_SCRIPT();

    void                SetUsers(USERS * u) { users = u; }
    void                SetSettings(const MODULE_SETTINGS & s) { settings = s; }
    int                 ParseSettings();

    int                 Start();
    int                 Stop();
    int                 Reload(const MODULE_SETTINGS & ms);
    bool                IsRunning() { return isRunning; }

    const std::string & GetStrError() const { return errorStr; }
    std::string         GetVersion() const { return "Remote script v 0.3"; }
    uint16_t            GetStartPosition() const { return 10; }
    uint16_t            GetStopPosition() const { return 10; }

    void                DelUser(USER_PTR u) { UnSetUserNotifiers(u); }
    void                AddUser(USER_PTR u) { SetUserNotifiers(u); }

    void                AddRSU(USER_PTR user);
    void                DelRSU(USER_PTR user);

private:
    REMOTE_SCRIPT(const REMOTE_SCRIPT & rhs);
    REMOTE_SCRIPT & operator=(const REMOTE_SCRIPT & rhs);

    static void *       Run(void *);
    bool                PrepareNet();
    bool                FinalizeNet();

    bool                Send(USER & rsu, bool forceDisconnect = false) const;
    bool                SendDirect(USER & rsu, uint32_t routerIP, bool forceDisconnect = false) const;
    bool                PreparePacket(char * buf, size_t bufSize, USER &rsu, bool forceDisconnect = false) const;
    void                PeriodicSend();

    std::vector<uint32_t> IP2Routers(uint32_t ip);
    bool                GetUsers();

    void                SetUserNotifiers(USER_PTR u);
    void                UnSetUserNotifiers(USER_PTR u);

    void                InitEncrypt(BLOWFISH_CTX * ctx, const std::string & password) const;
    void                Encrypt(BLOWFISH_CTX * ctx, void * dst, const void * src, size_t len8) const;

    mutable BLOWFISH_CTX ctx;

    std::list<IP_NOTIFIER> ipNotifierList;
    std::list<CONNECTED_NOTIFIER> connNotifierList;
    std::map<uint32_t, USER> authorizedUsers;

    mutable std::string errorStr;
    SETTINGS         rsSettings;
    MODULE_SETTINGS     settings;
    int                 sendPeriod;
    int                 halfPeriod;

    bool                nonstop;
    bool                isRunning;

    USERS *             users;

    std::vector<NET_ROUTER> netRouters;

    pthread_t           thread;
    pthread_mutex_t     mutex;

    int                 sock;

    ADD_USER_NONIFIER onAddUserNotifier;
    DEL_USER_NONIFIER onDelUserNotifier;

    PLUGIN_LOGGER       logger;

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
inline void ADD_USER_NONIFIER::Notify(const USER_PTR & user)
{
rs.AddUser(user);
}
//-----------------------------------------------------------------------------
inline void DEL_USER_NONIFIER::Notify(const USER_PTR & user)
{
rs.DelUser(user);
}
//-----------------------------------------------------------------------------

} // namespace RS

#endif
