#ifndef __RPC_CONFIG_H__
#define __RPC_CONFIG_H__

#include <pthread.h>

#include <ctime>
#include <string>
#include <map>
#include <vector>

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#include "stg/os_int.h"
#include "stg/plugin.h"
#include "stg/admin_conf.h"
#include "stg/module_settings.h"

#define RPC_CONFIG_VERSION "Stargazer RPC v. 0.2"

extern "C" PLUGIN * GetPlugin();

class ADMINS;
class TARIFFS;
class USERS;
class STORE;

class RPC_CONFIG_SETTINGS
{
public:
                         RPC_CONFIG_SETTINGS();
    virtual              ~RPC_CONFIG_SETTINGS() {};
    const std::string &  GetStrError() const { return errorStr; };
    int                  ParseSettings(const MODULE_SETTINGS & s);
    uint16_t             GetPort() const { return port; };
    double               GetCookieTimeout() const { return cookieTimeout; };
private:
    int     ParseIntInRange(const std::string & str,
                            int min,
                            int max,
                            int * val);
    std::string  errorStr;
    int          port;
    double       cookieTimeout;
};

struct ADMIN_INFO
{
    std::string admin;
    time_t      accessTime;
    PRIV        priviledges;
};

class RPC_CONFIG :public PLUGIN
{
public:
    RPC_CONFIG();
    virtual ~RPC_CONFIG();

    void                SetUsers(USERS * u) { users = u; }
    void                SetTariffs(TARIFFS * t) { tariffs = t; }
    void                SetAdmins(ADMINS * a) { admins = a; }
    void                SetStore(STORE * s) { store = s; }
    void                SetTraffcounter(TRAFFCOUNTER *) {}
    void                SetStgSettings(const SETTINGS * s);
    void                SetSettings(const MODULE_SETTINGS & s) { settings = s; }
    int                 ParseSettings();

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; }
    bool                IsRunning() { return running && !stopped; }

    const std::string & GetStrError() const { return errorStr; }
    const std::string   GetVersion() const { return RPC_CONFIG_VERSION; }
    uint16_t            GetStartPosition() const { return 220; }
    uint16_t            GetStopPosition() const { return 220; }

    bool                GetAdminInfo(const std::string & cookie,
                                     ADMIN_INFO * info);
    bool                CheckAdmin(const std::string & login,
                                   const std::string & password,
                                   std::string * cookie);
    bool                LogoutAdmin(const std::string & cookie);

private:
    static void *           Run(void *);
    std::string             GetCookie() const;
    void                    InitiateRegistry();

    mutable std::string     errorStr;
    RPC_CONFIG_SETTINGS     rpcConfigSettings;
    USERS *                 users;
    ADMINS *                admins;
    TARIFFS *               tariffs;
    STORE *                 store;
    MODULE_SETTINGS         settings;
    xmlrpc_c::registry      rpcRegistry;
    xmlrpc_c::serverAbyss * rpcServer;
    bool                    running;
    bool                    stopped;
    pthread_t               tid;
    std::map<std::string,
             ADMIN_INFO>    cookies;
    size_t                  dayFee;
    std::vector<std::string> dirNames;
};

#endif
