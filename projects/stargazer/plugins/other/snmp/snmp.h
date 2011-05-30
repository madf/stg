#ifndef __SNMP_AGENT_H__
#define __SNMP_AGENT_H__

#include <pthread.h>

#include <string>
#include <list>

#include "stg/os_int.h"
#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/users.h"

extern "C" PLUGIN * GetPlugin();

class USER;
class SETTINGS;
//-----------------------------------------------------------------------------
class SNMP_AGENT_SETTINGS {
public:
    SNMP_AGENT_SETTINGS();
    virtual ~SNMP_AGENT_SETTINGS() {}
    const std::string & GetStrError() const { return errorStr; }
    int ParseSettings(const MODULE_SETTINGS & s);

private:
    mutable std::string errorStr;
};
//-----------------------------------------------------------------------------
class SNMP_AGENT : public PLUGIN {
public:
    SNMP_AGENT();
    virtual ~SNMP_AGENT();

    void SetUsers(USERS *) {}
    void SetTariffs(TARIFFS *) {}
    void SetAdmins(ADMINS *) {}
    void SetTraffcounter(TRAFFCOUNTER *) {}
    void SetStore(STORE *) {}
    void SetStgSettings(const SETTINGS *) {}
    void SetSettings(const MODULE_SETTINGS &) {}
    int ParseSettings();

    int Start();
    int Stop();
    int Reload() { return 0; }
    bool IsRunning() { return running && !stopped; }

    const std::string & GetStrError() const { return errorStr; }
    const std::string GetVersion() const { return "Stg SNMP Agent 1.0"; }
    uint16_t GetStartPosition() const { return 100; }
    uint16_t GetStopPosition() const { return 100; }

private:
    static void * Runner(void * d);
    void Run();

    mutable std::string errorStr;
    SNMP_AGENT_SETTINGS snmpAgentSettings;
    MODULE_SETTINGS settings;

    pthread_t thread;
    pthread_mutex_t mutex;
    bool running;
    bool stopped;
};
//-----------------------------------------------------------------------------

#endif
