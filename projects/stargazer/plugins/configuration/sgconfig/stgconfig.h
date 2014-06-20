#ifndef STGCONFIG_H
#define STGCONFIG_H

#include "configproto.h"

#include "stg/plugin.h"
#include "stg/logger.h"

#include <string>

#include <pthread.h>

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

    void                SetUsers(USERS * users) { config.SetUsers(users); }
    void                SetTariffs(TARIFFS * tariffs) { config.SetTariffs(tariffs); }
    void                SetAdmins(ADMINS * admins) { config.SetAdmins(admins); }
    void                SetStore(STORE * store) { config.SetStore(store); }
    void                SetStgSettings(const SETTINGS * settings) { config.SetStgSettings(settings); }
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
    MODULE_SETTINGS     settings;
};
//-----------------------------------------------------------------------------

#endif
