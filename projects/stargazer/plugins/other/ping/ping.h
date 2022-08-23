#pragma once

#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/notifer.h"
#include "stg/subscriptions.h"
#include "stg/user_ips.h"
#include "stg/pinger.h"
#include "stg/users.h"
#include "stg/logger.h"

#include <string>
#include <list>
#include <mutex>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop
#include <cstdint>

class PING;

namespace STG
{
struct USER;
struct SETTINGS;
}

using UserPtr = STG::User*;
//-----------------------------------------------------------------------------*/
class CHG_CURRIP_NOTIFIER_PING: public STG::PropertyNotifierBase<uint32_t> {
public:
    CHG_CURRIP_NOTIFIER_PING(const PING & p, UserPtr u)
        : user(u), ping(p) {}
    void notify(const uint32_t & oldIP, const uint32_t & newIP) override;
    UserPtr GetUser() const { return user; }

private:
    CHG_CURRIP_NOTIFIER_PING & operator=(const CHG_CURRIP_NOTIFIER_PING &);

    UserPtr user;
    const PING & ping;
};
//-----------------------------------------------------------------------------
class CHG_IPS_NOTIFIER_PING: public STG::PropertyNotifierBase<STG::UserIPs> {
public:
    CHG_IPS_NOTIFIER_PING(const PING & p, UserPtr u)
        : user(u), ping(p) {}
    void notify(const STG::UserIPs & oldIPS, const STG::UserIPs & newIPS) override;
    UserPtr GetUser() const { return user; }

private:
    CHG_IPS_NOTIFIER_PING & operator=(const CHG_IPS_NOTIFIER_PING &);

    UserPtr user;
    const PING & ping;
};
//-----------------------------------------------------------------------------
class PING_SETTINGS {
public:
    PING_SETTINGS() : pingDelay(0) {}
    const std::string & GetStrError() const { return errorStr; }
    int ParseSettings(const STG::ModuleSettings & s);
    int GetPingDelay() const { return pingDelay; }
private:
    int pingDelay;
    mutable std::string errorStr;
};
//-----------------------------------------------------------------------------
class PING : public STG::Plugin {
friend class CHG_CURRIP_NOTIFIER_PING;
friend class CHG_IPS_NOTIFIER_PING;
public:
    PING();

    void SetUsers(STG::Users * u) override { users = u; }
    void SetSettings(const STG::ModuleSettings & s) override { settings = s; }
    int ParseSettings() override;

    int Start() override;
    int Stop() override;
    int Reload(const STG::ModuleSettings & /*ms*/) override { return 0; }
    bool IsRunning() override;

    const std::string & GetStrError() const override { return errorStr; }
    std::string GetVersion() const override { return "Pinger v.1.01"; }
    uint16_t GetStartPosition() const override { return 10; }
    uint16_t GetStopPosition() const override { return 10; }

    void AddUser(UserPtr u);
    void DelUser(UserPtr u);

private:
    explicit PING(const PING & rvalue);
    PING & operator=(const PING & rvalue);

    void GetUsers();
    void SetUserNotifiers(UserPtr u);
    void UnSetUserNotifiers(UserPtr u);
    void Run(std::stop_token token);

    mutable std::string errorStr;
    PING_SETTINGS pingSettings;
    STG::ModuleSettings settings;
    STG::Users * users;
    std::list<UserPtr> usersList;

    std::jthread m_thread;
    std::mutex m_mutex;
    bool isRunning;
    mutable STG_PINGER pinger;

    std::list<CHG_CURRIP_NOTIFIER_PING> ChgCurrIPNotifierList;
    std::list<CHG_IPS_NOTIFIER_PING> ChgIPNotifierList;

    STG::ScopedConnection m_onAddUserConn;
    STG::ScopedConnection m_onDelUserConn;

    STG::PluginLogger logger;
};
