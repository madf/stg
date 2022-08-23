#pragma once

#include "sensors.h"
#include "tables.h"
#include "types.h"

#include "stg/SMUX-PDUs.h"
#include "stg/ObjectSyntax.h"

#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/subscriptions.h"
#include "stg/notifer.h"
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
struct User;
struct Settings;
struct Users;
struct Tariffs;
struct Services;
struct Corporations;
struct TraffCounter;
}

class SMUX;

typedef bool (SMUX::*SMUXPacketHandler)(const SMUX_PDUs_t * pdus);
typedef bool (SMUX::*PDUsHandler)(const PDUs_t * pdus);
typedef std::map<SMUX_PDUs_PR, SMUXPacketHandler> SMUXHandlers;
typedef std::map<PDUs_PR, PDUsHandler> PDUsHandlers;

using UserPtr = STG::User*;
//-----------------------------------------------------------------------------
class SMUX_SETTINGS {
public:
    SMUX_SETTINGS();
    virtual ~SMUX_SETTINGS() {}
    const std::string & GetStrError() const { return errorStr; }
    int ParseSettings(const STG::ModuleSettings & s);

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
class CHG_AFTER_NOTIFIER : public STG::PropertyNotifierBase<std::string> {
public:
             CHG_AFTER_NOTIFIER(SMUX & s, const UserPtr & u)
                 : STG::PropertyNotifierBase<std::string>(),
                   smux(s), userPtr(u) {}
             CHG_AFTER_NOTIFIER(const CHG_AFTER_NOTIFIER & rvalue)
                 : STG::PropertyNotifierBase<std::string>(),
                   smux(rvalue.smux), userPtr(rvalue.userPtr) {}
    void     notify(const std::string &, const std::string &) override;

    UserPtr GetUserPtr() const { return userPtr; }

private:
    CHG_AFTER_NOTIFIER & operator=(const CHG_AFTER_NOTIFIER & rvalue);
    SMUX & smux;
    UserPtr userPtr;
};
//-----------------------------------------------------------------------------
class ADD_DEL_TARIFF_NOTIFIER : public STG::NotifierBase<STG::TariffData> {
public:
    explicit ADD_DEL_TARIFF_NOTIFIER(SMUX & s)
             : STG::NotifierBase<STG::TariffData>(), smux(s) {}
    void notify(const STG::TariffData &) override;

private:
    SMUX & smux;
};
//-----------------------------------------------------------------------------
class SMUX : public STG::Plugin {
public:
    SMUX();
    virtual ~SMUX();

    void SetUsers(STG::Users * u) { users = u; }
    void SetTariffs(STG::Tariffs * t) { tariffs = t; }
    void SetAdmins(STG::Admins * a) { admins = a; }
    void SetServices(STG::Services * s) { services = s; }
    void SetTraffcounter(STG::TraffCounter * tc) { traffcounter = tc; }
    void SetCorporations(STG::Corporations * c) { corporations = c; }
    void SetSettings(const STG::ModuleSettings & s) { settings = s; }
    int ParseSettings();

    int Start();
    int Stop();
    int Reload(const STG::ModuleSettings & ms);
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

    STG::Users * users;
    STG::Tariffs * tariffs;
    STG::Admins * admins;
    STG::Services * services;
    STG::Corporations * corporations;
    STG::TraffCounter * traffcounter;

    mutable std::string errorStr;
    SMUX_SETTINGS smuxSettings;
    STG::ModuleSettings settings;

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

    STG::ScopedConnection m_onAddUserConn;
    STG::ScopedConnection m_onDelUserConn;

    std::list<CHG_AFTER_NOTIFIER> notifiers;
    ADD_DEL_TARIFF_NOTIFIER addDelTariffNotifier;

    STG::PluginLogger logger;
};
//-----------------------------------------------------------------------------

inline
void CHG_AFTER_NOTIFIER::notify(const std::string &, const std::string &)
{
smux.UpdateTables();
}

inline
void ADD_DEL_TARIFF_NOTIFIER::notify(const STG::TariffData &)
{
smux.UpdateTables();
}
