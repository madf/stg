#ifndef __SMUX_H__
#define __SMUX_H__

#include <pthread.h>

#include <string>
#include <map>

#include "stg/SMUX-PDUs.h"
#include "stg/ObjectSyntax.h"

#include "stg/os_int.h"
#include "stg/plugin.h"
#include "stg/module_settings.h"

#include "sensors.h"
#include "tables.h"
#include "types.h"

extern "C" PLUGIN * GetPlugin();

class USER;
class SETTINGS;
class SMUX;
class USERS;
class TARIFFS;
class SERVICES;
class CORPORATIONS;

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
class SMUX : public PLUGIN {
public:
    SMUX();
    virtual ~SMUX();

    void SetUsers(USERS * u) { users = u; }
    void SetTariffs(TARIFFS * t) { tariffs = t; }
    void SetAdmins(ADMINS * a) { admins = a; }
    void SetServices(SERVICES * s) { services = s; }
    void SetCorporations(CORPORATIONS * c) { corporations = c; }
    void SetSettings(const MODULE_SETTINGS & s) { settings = s; }
    int ParseSettings();

    int Start();
    int Stop();
    int Reload() { return 0; }
    bool IsRunning() { return running && !stopped; }

    const std::string & GetStrError() const { return errorStr; }
    const std::string GetVersion() const { return "Stg SMUX Plugin 1.1"; }
    uint16_t GetStartPosition() const { return 100; }
    uint16_t GetStopPosition() const { return 100; }

private:
    static void * Runner(void * d);
    void Run();
    bool PrepareNet();

    bool DispatchPDUs(const SMUX_PDUs_t * pdus);

    bool CloseHandler(const SMUX_PDUs_t * pdus);
    bool RegisterResponseHandler(const SMUX_PDUs_t * pdus);
    bool PDUsRequestHandler(const SMUX_PDUs_t * pdus);
    bool CommitOrRollbackHandler(const SMUX_PDUs_t * pdus);

    bool GetRequestHandler(const PDUs_t * pdus);
    bool GetNextRequestHandler(const PDUs_t * pdus);
    bool SetRequestHandler(const PDUs_t * pdus);

    bool UpdateTables();

    USERS * users;
    TARIFFS * tariffs;
    ADMINS * admins;
    SERVICES * services;
    CORPORATIONS * corporations;

    mutable std::string errorStr;
    SMUX_SETTINGS smuxSettings;
    MODULE_SETTINGS settings;

    pthread_t thread;
    pthread_mutex_t mutex;
    bool running;
    bool stopped;

    int sock;

    SMUXHandlers smuxHandlers;
    PDUsHandlers pdusHandlers;
    Sensors sensors;
    Tables tables;

};
//-----------------------------------------------------------------------------

extern "C" PLUGIN * GetPlugin();

#endif
