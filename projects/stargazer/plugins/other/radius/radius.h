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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
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

#include <string>
#include <list>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include "os_int.h"
#include "base_auth.h"
#include "notifer.h"
#include "user_ips.h"
#include "../../../user.h"
#include "../../../users.h"
#include "blowfish.h"
#include "rad_packets.h"

using namespace std;

extern "C" BASE_PLUGIN * GetPlugin();

#define RAD_DEBUG (1)

class RADIUS;
//-----------------------------------------------------------------------------
class RAD_SETTINGS
{
public:
    virtual         ~RAD_SETTINGS(){};
    const string&   GetStrError() const { return errorStr; };
    int             ParseSettings(const MODULE_SETTINGS & s);
    uint16_t        GetPort() const;
    uint32_t        GetServerIP() const;
    int             GetPassword(string * password) const;
    int             GetAuthServices(list<string> * svcs) const;
    int             GetAcctServices(list<string> * svcs) const;

private:
    int             ParseIntInRange(const string & str, int min, int max, int * val);
    int             ParseIP(const string & str, uint32_t * routerIP);
    int             ParseServices(const vector<string> & str, list<string> * lst);

    uint16_t            port;
    string              errorStr;
    string              password;
    uint32_t            serverIP;
    list<string>        authServices;
    list<string>        acctServices;
};
//-----------------------------------------------------------------------------
struct RAD_SESSION {
    std::string userName;
    std::string serviceType;
};
//-----------------------------------------------------------------------------
class RADIUS :public BASE_AUTH
{
public:
                        RADIUS();
    virtual             ~RADIUS(){};

    void                SetUsers(USERS * u);
    void                SetTariffs(TARIFFS *){};
    void                SetAdmins(ADMINS *){};
    void                SetTraffcounter(TRAFFCOUNTER *){};
    void                SetStore(BASE_STORE * );
    void                SetStgSettings(const SETTINGS * s);
    void                SetSettings(const MODULE_SETTINGS & s);
    int                 ParseSettings();

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; };
    bool                IsRunning();

    const string      & GetStrError() const { return errorStr; };
    const string        GetVersion() const;
    uint16_t            GetStartPosition() const;
    uint16_t            GetStopPosition() const;

    int SendMessage(const STG_MSG &, uint32_t) const { return 0; };

private:
    static void *       Run(void *);
    int                 PrepareNet();
    int                 FinalizeNet();

    void                InitEncrypt(BLOWFISH_CTX * ctx, const string & password);
    void                Decrypt(BLOWFISH_CTX * ctx, char * dst, const char * src, int len8);
    void                Encrypt(BLOWFISH_CTX * ctx, char * dst, const char * src, int len8);

    int                 Send(const RAD_PACKET & packet);
    int                 RecvData(RAD_PACKET * packet);
    int                 ProcessData(RAD_PACKET * packet);

    int                 ProcessAutzPacket(RAD_PACKET * packet);
    int                 ProcessAuthPacket(RAD_PACKET * packet);
    int                 ProcessPostAuthPacket(RAD_PACKET * packet);
    int                 ProcessAcctStartPacket(RAD_PACKET * packet);
    int                 ProcessAcctStopPacket(RAD_PACKET * packet);
    int                 ProcessAcctUpdatePacket(RAD_PACKET * packet);
    int                 ProcessAcctOtherPacket(RAD_PACKET * packet);

    bool                FindUser(user_iter * ui, const std::string & login) const;
    bool                CanAuthService(const std::string & svc) const;
    bool                CanAcctService(const std::string & svc) const;
    bool                IsAllowedService(const std::string & svc) const;

    void                SetUserNotifier(user_iter u);
    void                UnSetUserNotifier(user_iter u);

    bool                WaitPackets(int sd) const;

    void                PrintServices(const std::list<std::string> & svcs);

    struct Printer : public unary_function<std::string, void>
    { 
        void operator()(const std::string & line)
        { 
            printfd("radius.cpp", "'%s'\n", line.c_str()); 
        }; 
    };
    struct SPrinter : public unary_function<std::pair<std::string, RAD_SESSION>, void>
    { 
        void operator()(const std::pair<std::string, RAD_SESSION> & it)
        { 
            printfd("radius.cpp", "%s - ('%s', '%s')\n", it.first.c_str(), it.second.userName.c_str(), it.second.serviceType.c_str()); 
        }; 
    };

    BLOWFISH_CTX        ctx;

    mutable string      errorStr;
    RAD_SETTINGS        radSettings;
    MODULE_SETTINGS     settings;
    list<string>        authServices;
    list<string>        acctServices;
    map<string, RAD_SESSION> sessions;

    bool                nonstop;

    bool                isRunning;

    USERS *             users;
    const SETTINGS *    stgSettings;
    const BASE_STORE *  store;

    pthread_t           thread;
    pthread_mutex_t     mutex;

    int                 sock;
    struct sockaddr_in  inAddr;
    socklen_t           inAddrLen;
    uint16_t            port;
    uint32_t            serverIP;
    struct sockaddr_in  outerAddr;
    socklen_t           outerAddrLen;

    RAD_PACKET          packet;

};
//-----------------------------------------------------------------------------

#endif

