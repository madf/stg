#pragma once

#include "stg/plugin.h"
#include "stg/admin_conf.h"
#include "stg/module_settings.h"
#include "stg/logger.h"

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#include <ctime>
#include <cstdint>
#include <string>
#include <map>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop

#define RPC_CONFIG_VERSION "Stargazer RPC v. 0.2"

namespace STG
{

struct Admins;
class Tariffs;
class Users;
struct Store;

}

class RPC_CONFIG_SETTINGS
{
public:
                         RPC_CONFIG_SETTINGS();
    virtual              ~RPC_CONFIG_SETTINGS() {}
    const std::string &  GetStrError() const { return errorStr; }
    int                  ParseSettings(const STG::ModuleSettings & s);
    uint16_t             GetPort() const { return port; }
    double               GetCookieTimeout() const { return cookieTimeout; }

private:
    std::string  errorStr;
    uint16_t     port;
    double       cookieTimeout;
};

struct ADMIN_INFO
{
    ADMIN_INFO()
        : admin(),
          accessTime(0),
          priviledges()
    {}

    std::string admin;
    time_t      accessTime;
    STG::Priv        priviledges;
};

class RPC_CONFIG : public STG::Plugin
{
public:
    RPC_CONFIG();
    ~RPC_CONFIG() override;

    void                SetUsers(STG::Users * u) override { users = u; }
    void                SetTariffs(STG::Tariffs * t) override { tariffs = t; }
    void                SetAdmins(STG::Admins * a) override { admins = a; }
    void                SetStore(STG::Store * s) override { store = s; }
    void                SetStgSettings(const STG::Settings * s) override;
    void                SetSettings(const STG::ModuleSettings & s) override { settings = s; }
    int                 ParseSettings() override;

    int                 Start() override;
    int                 Stop() override;
    int                 Reload(const STG::ModuleSettings & /*ms*/) override { return 0; }
    bool                IsRunning() override { return m_thread.joinable() && !stopped; }

    const std::string & GetStrError() const override { return errorStr; }
    std::string         GetVersion() const override { return RPC_CONFIG_VERSION; }
    uint16_t            GetStartPosition() const override { return 20; }
    uint16_t            GetStopPosition() const override { return 20; }

    bool                GetAdminInfo(const std::string & cookie,
                                     ADMIN_INFO * info);
    bool                CheckAdmin(const std::string & login,
                                   const std::string & password,
                                   std::string * cookie);
    bool                LogoutAdmin(const std::string & cookie);

private:
    RPC_CONFIG(const RPC_CONFIG & rvalue);
    RPC_CONFIG & operator=(const RPC_CONFIG & rvalue);

    void                    Run(std::stop_token token);
    std::string             GetCookie() const;
    void                    InitiateRegistry();

    mutable std::string     errorStr;
    RPC_CONFIG_SETTINGS     rpcConfigSettings;
    STG::Users *                 users;
    STG::Admins *                admins;
    STG::Tariffs *               tariffs;
    STG::Store *                 store;
    STG::ModuleSettings         settings;
    int                     fd;
    xmlrpc_c::registry      rpcRegistry;
    xmlrpc_c::serverAbyss * rpcServer;
    bool                    stopped;
    std::jthread            m_thread;
    std::map<std::string,
             ADMIN_INFO>    cookies;
    size_t                  dayFee;
    std::vector<std::string> dirNames;
    STG::PluginLogger           logger;
};
