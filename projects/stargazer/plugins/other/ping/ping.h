#pragma once

#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/subscriptions.h"
#include "stg/user_ips.h"
#include "stg/pinger.h"
#include "stg/users.h"
#include "stg/logger.h"

#include <string>
#include <vector>
#include <tuple>
#include <list>
#include <mutex>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop
#include <cstdint>

namespace STG
{
struct USER;
struct SETTINGS;

using UserPtr = User*;
//-----------------------------------------------------------------------------
class PING_SETTINGS
{
    public:
        PING_SETTINGS() : pingDelay(0) {}
        const std::string & GetStrError() const { return errorStr; }
        int ParseSettings(const ModuleSettings & s);
        int GetPingDelay() const { return pingDelay; }
    private:
        int pingDelay;
        std::string errorStr;
};
//-----------------------------------------------------------------------------
class PING : public Plugin
{
    public:
        PING();

        void SetUsers(Users * u) override { users = u; }
        void SetSettings(const ModuleSettings & s) override { settings = s; }
        int ParseSettings() override;

        int Start() override;
        int Stop() override;
        int Reload(const ModuleSettings & /*ms*/) override { return 0; }
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

        std::string errorStr;
        PING_SETTINGS pingSettings;
        ModuleSettings settings;
        Users * users;
        std::list<UserPtr> usersList;

        std::jthread m_thread;
        std::mutex m_mutex;
        bool isRunning;
        STG_PINGER m_pinger;

        void updateCurrIP(uint32_t oldVal, uint32_t newVal);
        void updateIPs(const UserIPs& oldVal, const UserIPs& newVal);

        ScopedConnection m_onAddUserConn;
        ScopedConnection m_onDelUserConn;

        using ConnHolder = std::tuple<int, ScopedConnection, ScopedConnection>;
        std::vector<ConnHolder> m_conns;

        PluginLogger logger;
};

}
