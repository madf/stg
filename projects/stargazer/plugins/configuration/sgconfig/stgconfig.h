#ifndef STGCONFIG_H
#define STGCONFIG_H

#include <pthread.h>

#include <string>

#include "stg/plugin.h"
#include "stg/store.h"
#include "stg/logger.h"
#include "configproto.h"

extern "C" PLUGIN * GetPlugin();

class STG_CONFIG;

class STG_CONFIG_SETTINGS {
public:
                    STG_CONFIG_SETTINGS() : errorStr(), port(0) {}
    virtual         ~STG_CONFIG_SETTINGS() {}
    const std::string & GetStrError() const { return errorStr; }
    int             ParseSettings(const MODULE_SETTINGS & s);
    uint16_t        GetPort() const { return port; }
private:
    std::string errorStr;
    uint16_t    port;
};
//-----------------------------------------------------------------------------
class STG_CONFIG :public PLUGIN {
public:
    STG_CONFIG();
    virtual ~STG_CONFIG(){}

    void                SetUsers(USERS * u) { users = u; }
    void                SetTariffs(TARIFFS * t) { tariffs = t; }
    void                SetAdmins(ADMINS * a) { admins = a; }
    void                SetStore(STORE * s) { store = s; }
    void                SetStgSettings(const SETTINGS * s) { stgSettings = s; }
    void                SetSettings(const MODULE_SETTINGS & s) { settings = s; }
    int                 ParseSettings();

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; }
    bool                IsRunning() { return isRunning; }

    const std::string & GetStrError() const { return errorStr; }
    std::string         GetVersion() const;
    uint16_t            GetStartPosition() const { return 20; }
    uint16_t            GetStopPosition() const { return 20; }

private:
    STG_CONFIG(const STG_CONFIG & rvalue);
    STG_CONFIG & operator=(const STG_CONFIG & rvalue);

    static void *       Run(void *);

    mutable std::string errorStr;
    STG_CONFIG_SETTINGS stgConfigSettings;
    pthread_t           thread;
    bool                nonstop;
    bool                isRunning;
    PLUGIN_LOGGER       logger;
    CONFIGPROTO         config;
    USERS *             users;
    ADMINS *            admins;
    TARIFFS *           tariffs;
    STORE *             store;
    MODULE_SETTINGS     settings;
    const SETTINGS *    stgSettings;
};
//-----------------------------------------------------------------------------

#endif
