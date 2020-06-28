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
 *    Author : Maksym Mamontov <stg@madf.info>
 */

/*
 *  Radius data access plugin for Stargazer
 *
 *  $Revision: 1.10 $
 *  $Date: 2009/12/13 14:17:13 $
 *
 */

#ifndef RADIUS_H
#define RADIUS_H

#include <pthread.h>

#include <cstring>
#include <cstdlib>
#include <string>
#include <list>
#include <map>
#include <vector>

#include "stg/os_int.h"
#include "stg/auth.h"
#include "stg/module_settings.h"
#include "stg/notifer.h"
#include "stg/user_ips.h"
#include "stg/user.h"
#include "stg/users.h"
#include "stg/blowfish.h"
#include "stg/rad_packets.h"
#include "stg/logger.h"

extern "C" PLUGIN * GetPlugin();

#define RAD_DEBUG (1)

class RADIUS;
//-----------------------------------------------------------------------------
class RAD_SETTINGS {
public:
    RAD_SETTINGS()
        : port(0), errorStr(), password(),
          authServices(), acctServices()
    {}
    virtual ~RAD_SETTINGS() {}
    const std::string & GetStrError() const { return errorStr; }
    int ParseSettings(const MODULE_SETTINGS & s);
    uint16_t GetPort() const { return port; }
    const std::string & GetPassword() const { return password; }
    const std::list<std::string> & GetAuthServices() const { return authServices; }
    const std::list<std::string> & GetAcctServices() const { return acctServices; }

private:
    int ParseServices(const std::vector<std::string> & str, std::list<std::string> * lst);

    uint16_t port;
    std::string errorStr;
    std::string password;
    std::list<std::string> authServices;
    std::list<std::string> acctServices;
};
//-----------------------------------------------------------------------------
struct RAD_SESSION {
    RAD_SESSION() : userName(), serviceType() {}
    std::string userName;
    std::string serviceType;
};
//-----------------------------------------------------------------------------
class RADIUS :public AUTH {
public:
                        RADIUS();
    virtual             ~RADIUS() {}

    void                SetUsers(USERS * u) { users = u; }
    void                SetStore(STORE * s) { store = s; }
    void                SetStgSettings(const SETTINGS *) {}
    void                SetSettings(const MODULE_SETTINGS & s) { settings = s; }
    int                 ParseSettings();

    int                 Start();
    int                 Stop();
    int                 Reload(const MODULE_SETTINGS & /*ms*/) { return 0; }
    bool                IsRunning() { return isRunning; }

    const std::string & GetStrError() const { return errorStr; }
    std::string         GetVersion() const { return "RADIUS data access plugin v 0.6"; }
    uint16_t            GetStartPosition() const { return 30; }
    uint16_t            GetStopPosition() const { return 30; }

    int SendMessage(const STG_MSG &, uint32_t) const { return 0; }

private:
    RADIUS(const RADIUS & rvalue);
    RADIUS & operator=(const RADIUS & rvalue);

    static void *       Run(void *);
    int                 PrepareNet();
    int                 FinalizeNet();

    ssize_t             Send(const RAD_PACKET & packet, struct sockaddr_in * outerAddr);
    int                 RecvData(RAD_PACKET * packet, struct sockaddr_in * outerAddr);
    int                 ProcessData(RAD_PACKET * packet);

    int                 ProcessAutzPacket(RAD_PACKET * packet);
    int                 ProcessAuthPacket(RAD_PACKET * packet);
    int                 ProcessPostAuthPacket(RAD_PACKET * packet);
    int                 ProcessAcctStartPacket(RAD_PACKET * packet);
    int                 ProcessAcctStopPacket(RAD_PACKET * packet);
    int                 ProcessAcctUpdatePacket(RAD_PACKET * packet);
    int                 ProcessAcctOtherPacket(RAD_PACKET * packet);

    bool                FindUser(USER_PTR * ui, const std::string & login) const;
    bool                CanAuthService(const std::string & svc) const;
    bool                CanAcctService(const std::string & svc) const;
    bool                IsAllowedService(const std::string & svc) const;

    struct SPrinter : public std::unary_function<std::pair<std::string, RAD_SESSION>, void>
    {
        void operator()(const std::pair<std::string, RAD_SESSION> & it)
        {
            printfd("radius.cpp", "%s - ('%s', '%s')\n", it.first.c_str(), it.second.userName.c_str(), it.second.serviceType.c_str());
        }
    };

    BLOWFISH_CTX        ctx;

    mutable std::string errorStr;
    RAD_SETTINGS        radSettings;
    MODULE_SETTINGS     settings;
    std::list<std::string> authServices;
    std::list<std::string> acctServices;
    std::map<std::string, RAD_SESSION> sessions;

    bool                nonstop;
    bool                isRunning;

    USERS *             users;
    const SETTINGS *    stgSettings;
    const STORE *       store;

    pthread_t           thread;
    pthread_mutex_t     mutex;

    int                 sock;

    RAD_PACKET          packet;

    PLUGIN_LOGGER       logger;
};
//-----------------------------------------------------------------------------

#endif
