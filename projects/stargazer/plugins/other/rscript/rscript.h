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

/*
 $Revision: 1.16 $
 $Date: 2010/09/10 06:43:59 $
 $Author: faust $
*/

#ifndef RSCRIPT_H
#define RSCRIPT_H

#include <pthread.h>

#include <cstring>
#include <string>
#include <list>
#include <map>
#include <functional>
#include <utility>

#include "stg/plugin.h"
#include "stg/store.h"
#include "stg/module_settings.h"
#include "stg/os_int.h"
#include "stg/notifer.h"
#include "stg/user_ips.h"
#include "stg/user.h"
#include "stg/users.h"
#include "stg/blowfish.h"
#include "stg/rs_packets.h"
#include "nrmap_parser.h"

extern "C" PLUGIN * GetPlugin();

#define RS_DEBUG (1)

#define MAX_SHORT_PCKT  (3)

class REMOTE_SCRIPT;
class SETTINGS;
//-----------------------------------------------------------------------------
class RS_ADD_USER_NONIFIER: public NOTIFIER_BASE<USER_PTR> {
public:
    RS_ADD_USER_NONIFIER(REMOTE_SCRIPT & r)
        : NOTIFIER_BASE<USER_PTR>(), rs(r) {}
    virtual ~RS_ADD_USER_NONIFIER() {}
    void Notify(const USER_PTR & user);

private:
    RS_ADD_USER_NONIFIER(const RS_ADD_USER_NONIFIER & rvalue);
    RS_ADD_USER_NONIFIER & operator=(const RS_ADD_USER_NONIFIER);

    REMOTE_SCRIPT & rs;
};
//-----------------------------------------------------------------------------
class RS_DEL_USER_NONIFIER: public NOTIFIER_BASE<USER_PTR> {
public:
    RS_DEL_USER_NONIFIER(REMOTE_SCRIPT & r)
        : NOTIFIER_BASE<USER_PTR>(), rs(r) {}
    virtual ~RS_DEL_USER_NONIFIER() {}
    void Notify(const USER_PTR & user);

private:
    RS_DEL_USER_NONIFIER(const RS_DEL_USER_NONIFIER & rvalue);
    RS_DEL_USER_NONIFIER & operator=(const RS_DEL_USER_NONIFIER);

    REMOTE_SCRIPT & rs;
};
//-----------------------------------------------------------------------------
template <typename T>
class RS_CHG_AFTER_NOTIFIER: public PROPERTY_NOTIFIER_BASE<T> {
public:
    RS_CHG_AFTER_NOTIFIER(REMOTE_SCRIPT & r, USER_PTR u)
        : PROPERTY_NOTIFIER_BASE<T>(), user(u), rs(r) {}
    RS_CHG_AFTER_NOTIFIER(const RS_CHG_AFTER_NOTIFIER<T> & rvalue)
        : PROPERTY_NOTIFIER_BASE<T>(), user(rvalue.user), rs(rvalue.rs) {}
    void Notify(const T & oldValue, const T & newValue);
    USER_PTR GetUser() { return user; }

private:
    RS_CHG_AFTER_NOTIFIER<T> & operator=(const RS_CHG_AFTER_NOTIFIER<T> & rvalue);

    USER_PTR user;
    REMOTE_SCRIPT & rs;
};
//-----------------------------------------------------------------------------
struct RS_USER {
    RS_USER()
        : lastSentTime(0),
          user(NULL),
          routers(),
          shortPacketsCount(0)
    {}
    RS_USER(const std::vector<uint32_t> & r, USER_PTR it)
        : lastSentTime(0),
          user(it),
          routers(r),
          shortPacketsCount(0)
    {}
    RS_USER(const RS_USER & rvalue)
        : lastSentTime(rvalue.lastSentTime),
          user(rvalue.user),
          routers(rvalue.routers),
          shortPacketsCount(rvalue.shortPacketsCount)
    {}

    RS_USER & operator=(const RS_USER & rvalue);

    time_t lastSentTime;
    USER_PTR user;
    std::vector<uint32_t> routers;
    int shortPacketsCount;
};
//-----------------------------------------------------------------------------
class RS_SETTINGS {
public:
                        RS_SETTINGS();
    virtual             ~RS_SETTINGS() {}
    const std::string & GetStrError() const { return errorStr; }
    int                 ParseSettings(const MODULE_SETTINGS & s);
    int                 GetSendPeriod() const { return sendPeriod; }
    int                 GetPort() const { return port; }
    const std::vector<NET_ROUTER> & GetSubnetsMap() const { return netRouters; }
    const std::vector<std::string> & GetUserParams() const { return userParams; }
    const std::string & GetPassword() const { return password; }
    const std::string & GetMapFileName() const { return subnetFile; }

private:
    int                 sendPeriod;
    uint16_t            port;
    string              errorStr;
    std::vector<NET_ROUTER> netRouters;
    std::vector<string> userParams;
    string              password;
    string              subnetFile;
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
    int                 Reload();
    bool                IsRunning() { return isRunning; }

    const std::string & GetStrError() const { return errorStr; }
    const std::string   GetVersion() const { return "Remote script v 0.3"; }
    uint16_t            GetStartPosition() const { return 10; }
    uint16_t            GetStopPosition() const { return 10; }

    void                DelUser(USER_PTR u) { UnSetUserNotifier(u); }
    void                AddUser(USER_PTR u) { SetUserNotifier(u); }

    void                ChangedIP(USER_PTR u, uint32_t oldIP, uint32_t newIP);

private:
    REMOTE_SCRIPT(const REMOTE_SCRIPT & rvalue);
    REMOTE_SCRIPT & operator=(const REMOTE_SCRIPT & rvalue);

    static void *       Run(void *);
    bool                PrepareNet();
    bool                FinalizeNet();

    bool                Send(uint32_t ip, RS_USER & rsu, bool forceDisconnect = false) const;
    bool                SendDirect(uint32_t ip, RS_USER & rsu, uint32_t routerIP, bool forceDisconnect = false) const;
    bool                PreparePacket(char * buf, size_t bufSize, uint32_t ip, RS_USER &rsu, bool forceDisconnect = false) const;
    void                PeriodicSend();

    std::vector<uint32_t> IP2Routers(uint32_t ip);
    bool                GetUsers();
    std::string         GetUserParam(USER_PTR u, const std::string & paramName) const;

    void                SetUserNotifier(USER_PTR u);
    void                UnSetUserNotifier(USER_PTR u);

    void                InitEncrypt(BLOWFISH_CTX * ctx, const string & password) const;
    void                Encrypt(BLOWFISH_CTX * ctx, char * dst, const char * src, size_t len8) const;

    mutable BLOWFISH_CTX ctx;

    std::list<RS_CHG_AFTER_NOTIFIER<uint32_t> > afterChgIPNotifierList;
    std::map<uint32_t, RS_USER> authorizedUsers;

    mutable std::string errorStr;
    RS_SETTINGS         rsSettings;
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

    RS_ADD_USER_NONIFIER onAddUserNotifier;
    RS_DEL_USER_NONIFIER onDelUserNotifier;

    friend class UpdateRouter;
    friend class DisconnectUser;
};
//-----------------------------------------------------------------------------
class DisconnectUser : public std::unary_function<std::pair<const uint32_t, RS_USER> &, void> {
    public:
        DisconnectUser(REMOTE_SCRIPT & rs) : rscript(rs) {}
        void operator()(std::pair<const uint32_t, RS_USER> & p)
        {
            rscript.Send(p.first, p.second, true);
        }
    private:
        REMOTE_SCRIPT & rscript;
};
//-----------------------------------------------------------------------------
inline void RS_ADD_USER_NONIFIER::Notify(const USER_PTR & user)
{
rs.AddUser(user);
}
//-----------------------------------------------------------------------------
inline void RS_DEL_USER_NONIFIER::Notify(const USER_PTR & user)
{
rs.DelUser(user);
}
//-----------------------------------------------------------------------------

#endif
