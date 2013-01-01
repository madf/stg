#ifndef __SMUX_H__
#define __SMUX_H__

#include <pthread.h>

#include <string>
#include <map>
#include <list>

#include "stg/SMUX-PDUs.h"
#include "stg/ObjectSyntax.h"

#include "stg/os_int.h"
#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/notifer.h"
#include "stg/noncopyable.h"
#include "stg/logger.h"

#include "sensors.h"
#include "tables.h"
#include "types.h"

class USER;
class SETTINGS;
class SMUX;
class USERS;
class TARIFFS;
class SERVICES;
class CORPORATIONS;
class TRAFFCOUNTER;

typedef bool (SMUX::*SMUXPacketHandler)(const SMUX_PDUs_t * pdus);
typedef bool (SMUX::*PDUsHandler)(const PDUs_t * pdus);
typedef std::map<SMUX_PDUs_PR, SMUXPacketHandler> SMUXHandlers;
typedef std::map<PDUs_PR, PDUsHandler> PDUsHandlers;
//-----------------------------------------------------------------------------
class SMUX_SETTINGS {
public:
    SMUX_SETTINGS();
    virtual ~SMUX_SETTINGS() {}
    const std::string & GetStrError() const { return errorStr; }
    int ParseSettings(const MODULE_SETTINGS & s);

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
class CHG_AFTER_NOTIFIER : public PROPERTY_NOTIFIER_BASE<std::string> {
public:
             CHG_AFTER_NOTIFIER(SMUX & s, const USER_PTR & u)
                 : PROPERTY_NOTIFIER_BASE<std::string>(),
                   smux(s), userPtr(u) {}
             CHG_AFTER_NOTIFIER(const CHG_AFTER_NOTIFIER & rvalue)
                 : PROPERTY_NOTIFIER_BASE<std::string>(),
                   smux(rvalue.smux), userPtr(rvalue.userPtr) {}
    void     Notify(const std::string &, const std::string &);

    USER_PTR GetUserPtr() const { return userPtr; }

private:
    CHG_AFTER_NOTIFIER & operator=(const CHG_AFTER_NOTIFIER & rvalue);
    SMUX & smux;
    USER_PTR userPtr;
};
//-----------------------------------------------------------------------------
class ADD_DEL_TARIFF_NOTIFIER : public NOTIFIER_BASE<TARIFF_DATA>, private NONCOPYABLE {
public:
         ADD_DEL_TARIFF_NOTIFIER(SMUX & s)
             : NOTIFIER_BASE<TARIFF_DATA>(), smux(s) {}
    void Notify(const TARIFF_DATA &);

private:
    SMUX & smux;
};
//-----------------------------------------------------------------------------
class ADD_USER_NOTIFIER : public NOTIFIER_BASE<USER_PTR>, private NONCOPYABLE {
public:
         ADD_USER_NOTIFIER(SMUX & s) : NOTIFIER_BASE<USER_PTR>(), smux(s) {}
    void Notify(const USER_PTR &);

private:
    SMUX & smux;
};
//-----------------------------------------------------------------------------
class DEL_USER_NOTIFIER : public NOTIFIER_BASE<USER_PTR>, private NONCOPYABLE {
public:
         DEL_USER_NOTIFIER(SMUX & s) : NOTIFIER_BASE<USER_PTR>(), smux(s) {}
    void Notify(const USER_PTR &);

private:
    SMUX & smux;
};
//-----------------------------------------------------------------------------
class SMUX : public PLUGIN {
public:
    SMUX();
    virtual ~SMUX();

    void SetUsers(USERS * u) { users = u; }
    void SetTariffs(TARIFFS * t) { tariffs = t; }
    void SetAdmins(ADMINS * a) { admins = a; }
    void SetServices(SERVICES * s) { services = s; }
    void SetTraffcounter(TRAFFCOUNTER * tc) { traffcounter = tc; }
    void SetCorporations(CORPORATIONS * c) { corporations = c; }
    void SetSettings(const MODULE_SETTINGS & s) { settings = s; }
    int ParseSettings();

    int Start();
    int Stop();
    int Reload();
    bool IsRunning() { return running && !stopped; }

    const std::string & GetStrError() const { return errorStr; }
    const std::string GetVersion() const { return "Stg SMUX Plugin 1.1"; }
    uint16_t GetStartPosition() const { return 10; }
    uint16_t GetStopPosition() const { return 10; }

    bool UpdateTables();

    void SetNotifier(USER_PTR userPtr);
    void UnsetNotifier(USER_PTR userPtr);

private:
    SMUX(const SMUX & rvalue);
    SMUX & operator=(const SMUX & rvalue);

    static void * Runner(void * d);
    void Run();
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

    USERS * users;
    TARIFFS * tariffs;
    ADMINS * admins;
    SERVICES * services;
    CORPORATIONS * corporations;
    TRAFFCOUNTER * traffcounter;

    mutable std::string errorStr;
    SMUX_SETTINGS smuxSettings;
    MODULE_SETTINGS settings;

    pthread_t thread;
    pthread_mutex_t mutex;
    bool running;
    bool stopped;
    bool needReconnect;

    time_t lastReconnectTry;
    unsigned reconnectTimeout;

    int sock;

    SMUXHandlers smuxHandlers;
    PDUsHandlers pdusHandlers;
    Sensors sensors;
    Tables tables;

    std::list<CHG_AFTER_NOTIFIER> notifiers;
    ADD_USER_NOTIFIER addUserNotifier;
    DEL_USER_NOTIFIER delUserNotifier;
    ADD_DEL_TARIFF_NOTIFIER addDelTariffNotifier;

    PLUGIN_LOGGER logger;
};
//-----------------------------------------------------------------------------

inline
void CHG_AFTER_NOTIFIER::Notify(const std::string &, const std::string &)
{
smux.UpdateTables();
}

inline
void ADD_DEL_TARIFF_NOTIFIER::Notify(const TARIFF_DATA &)
{
smux.UpdateTables();
}

inline
void ADD_USER_NOTIFIER::Notify(const USER_PTR & userPtr)
{
smux.SetNotifier(userPtr);
smux.UpdateTables();
}

inline
void DEL_USER_NOTIFIER::Notify(const USER_PTR & userPtr)
{
smux.UnsetNotifier(userPtr);
smux.UpdateTables();
}

#endif
