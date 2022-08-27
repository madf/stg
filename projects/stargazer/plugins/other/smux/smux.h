#pragma once

#include "sensors.h"
#include "tables.h"
#include "types.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include "stg/SMUX-PDUs.h"
#include "stg/ObjectSyntax.h"
#pragma GCC diagnostic pop

#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/subscriptions.h"
#include "stg/noncopyable.h"
#include "stg/logger.h"

#include <string>
#include <map>
#include <list>
#include <mutex>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop
#include <cstdint>

namespace STG
{
class User;
struct Settings;
class Users;
class Tariffs;
struct Services;
struct Corporations;
struct TraffCounter;

class SMUX;

typedef bool (SMUX::*SMUXPacketHandler)(const SMUX_PDUs_t * pdus);
typedef bool (SMUX::*PDUsHandler)(const PDUs_t * pdus);
typedef std::map<SMUX_PDUs_PR, SMUXPacketHandler> SMUXHandlers;
typedef std::map<PDUs_PR, PDUsHandler> PDUsHandlers;

using UserPtr = User*;
//-----------------------------------------------------------------------------
class SMUX_SETTINGS
{
    public:
        SMUX_SETTINGS();
        virtual ~SMUX_SETTINGS() {}
        const std::string & GetStrError() const { return errorStr; }
        int ParseSettings(const ModuleSettings & s);

        uint32_t GetIP() const { return ip; }
        uint16_t GetPort() const { return port; }
        const std::string GetPassword() const { return password; }

    private:
        mutable std::string errorStr;

        uint32_t ip;
        uint16_t port;
        std::string password;
};
//-----------------------------------------------------------------------------
class SMUX : public Plugin
{
    public:
        SMUX();
        virtual ~SMUX();

        void SetUsers(Users * u) { users = u; }
        void SetTariffs(Tariffs * t) { tariffs = t; }
        void SetAdmins(Admins * a) { admins = a; }
        void SetServices(Services * s) { services = s; }
        void SetTraffcounter(TraffCounter * tc) { traffcounter = tc; }
        void SetCorporations(Corporations * c) { corporations = c; }
        void SetSettings(const ModuleSettings & s) { settings = s; }
        int ParseSettings();

        int Start();
        int Stop();
        int Reload(const ModuleSettings & ms);
        bool IsRunning() { return m_thread.joinable() && !stopped; }

        const std::string & GetStrError() const { return errorStr; }
        std::string GetVersion() const { return "Stg SMUX Plugin 1.1"; }
        uint16_t GetStartPosition() const { return 10; }
        uint16_t GetStopPosition() const { return 10; }

        bool UpdateTables();

        void SetNotifier(UserPtr userPtr);
        void UnsetNotifier(UserPtr userPtr);

    private:
        SMUX(const SMUX & rvalue);
        SMUX & operator=(const SMUX & rvalue);

        void Run(std::stop_token token);
        bool PrepareNet();
        bool Reconnect();

        bool DispatchPDUs(const SMUX_PDUs_t * pdus);

        bool CloseHandler(const SMUX_PDUs_t * pdus);
        bool RegisterResponseHandler(const SMUX_PDUs_t * pdus);
        bool PDUsRequestHandler(const SMUX_PDUs_t * pdus);
        bool CommitOrRollbackHandler(const SMUX_PDUs_t * pdus);

        bool GetRequestHandler(const PDUs_t * pdus);
        bool GetNextRequestHandler(const PDUs_t * pdus);
        bool SetRequestHandler(const PDUs_t * pdus);

        void SetNotifiers();
        void ResetNotifiers();

        Users * users;
        Tariffs * tariffs;
        Admins * admins;
        Services * services;
        Corporations * corporations;
        TraffCounter * traffcounter;

        mutable std::string errorStr;
        SMUX_SETTINGS smuxSettings;
        ModuleSettings settings;

        std::jthread m_thread;
        std::mutex m_mutex;
        bool stopped;
        bool needReconnect;

        time_t lastReconnectTry;
        unsigned reconnectTimeout;

        int sock;

        SMUXHandlers smuxHandlers;
        PDUsHandlers pdusHandlers;
        Sensors sensors;
        Tables tables;

        ScopedConnection m_onAddUserConn;
        ScopedConnection m_onDelUserConn;
        ScopedConnection m_onAddTariffConn;
        ScopedConnection m_onDelTariffConn;

        using ConnHolder = std::tuple<int, ScopedConnection>;
        std::vector<ConnHolder> m_conns;

        PluginLogger logger;
};

}
