#ifndef __STGCONFIG_H__
#define __STGCONFIG_H__

#include <string>

#include "plugin.h"
#include "os_int.h"
#include "main_thread.h"

#define PLUGIN_VERSION "stgconfig v.2.0"

namespace boost {
    class thread;
};

class USERS;
class TARIFFS;
class ADMINS;
class STORE;
class TRAFFCOUNTER;
class SETTINGS;

class STGCONFIG2 : public PLUGIN {
public:
    STGCONFIG2();
    virtual ~STGCONFIG2();

    void                SetUsers(USERS * u) { users = u; }
    void                SetTariffs(TARIFFS * t) { tariffs = t; }
    void                SetAdmins(ADMINS * a) { admins = a; }
    void                SetStore(STORE * s) { store = s; }
    void                SetStgSettings(const SETTINGS * s) { stgSettings = s; }
    void                SetSettings(const MODULE_SETTINGS & s) { modSettings = s; }
    int                 ParseSettings();

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; }
    bool                IsRunning();

    const std::string & GetStrError() const { return errorStr; }
    const std::string   GetVersion() const { return PLUGIN_VERSION; }
    uint16_t            GetStartPosition() const { return 20; }
    uint16_t            GetStopPosition() const { return 20; }

private:
    USERS * users;
    TARIFFS * tariffs;
    ADMINS * admins;
    STORE * store;
    const SETTINGS * stgSettings;
    MODULE_SETTINGS modSettings;

    MAIN_THREAD ct;

    mutable std::string errorStr;

    boost::thread * thread;

};

extern "C" PLUGIN * GetPlugin();

#endif
