#ifndef STGCONFIG_H
#define STGCONFIG_H

#include <pthread.h>

#include <string>

#include "plugin.h"
#include "store.h"
#include "configproto.h"

extern "C" PLUGIN * GetPlugin();

class STG_CONFIG;

class STG_CONFIG_SETTINGS {
public:
                    STG_CONFIG_SETTINGS();
    virtual         ~STG_CONFIG_SETTINGS() {}
    const std::string & GetStrError() const;
    int             ParseSettings(const MODULE_SETTINGS & s);
    uint16_t        GetPort() const;
private:
    int     ParseIntInRange(const std::string & str, int min, int max, int * val);
    std::string errorStr;
    int     port;
};
//-----------------------------------------------------------------------------
class STG_CONFIG :public PLUGIN {
public:
    STG_CONFIG();
    virtual ~STG_CONFIG(){};

    void                SetUsers(USERS * u);
    void                SetTariffs(TARIFFS * t);
    void                SetAdmins(ADMINS * a);
    void                SetStore(STORE * s);
    void                SetTraffcounter(TRAFFCOUNTER *) {}
    void                SetStgSettings(const SETTINGS * s);
    void                SetSettings(const MODULE_SETTINGS & s);
    int                 ParseSettings();

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; }
    bool                IsRunning();

    const std::string & GetStrError() const;
    const std::string   GetVersion() const;
    uint16_t            GetStartPosition() const;
    uint16_t            GetStopPosition() const;

private:
    static void *       Run(void *);
    mutable std::string errorStr;
    STG_CONFIG_SETTINGS stgConfigSettings;
    pthread_t           thread;
    bool                nonstop;
    bool                isRunning;
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
