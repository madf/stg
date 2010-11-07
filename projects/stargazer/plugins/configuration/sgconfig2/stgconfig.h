#include <pthread.h>

#include <string>
#include <list>

#include "base_plugin.h"
#include "base_store.h"
#include "configproto.h"

using namespace std;

extern "C" BASE_PLUGIN * GetPlugin();

class STG_CONFIG;

//-----------------------------------------------------------------------------
class STG_CONFIG_SETTINGS
{
public:
                    STG_CONFIG_SETTINGS();
    virtual         ~STG_CONFIG_SETTINGS(){};
    const string &  GetStrError() const;
    int             ParseSettings(const MODULE_SETTINGS & s);
    uint16_t        GetPort();
private:
    int     ParseIntInRange(const string & str, int min, int max, int * val);
    string  errorStr;
    int     port;
};
//-----------------------------------------------------------------------------
class STG_CONFIG: public BASE_PLUGIN
{
public:
    STG_CONFIG();
    virtual ~STG_CONFIG(){};

    void                SetUsers(USERS * u) { users = u; };
    void                SetTariffs(TARIFFS * t) { tariffs = t; };
    void                SetAdmins(ADMINS * a) { admins = a; };
    void                SetStore(BASE_STORE * s) { store = s; };
    void                SetTraffcounter(TRAFFCOUNTER *) {};
    void                SetStgSettings(const SETTINGS * s) { stgConfigSettings = s; };
    void                SetSettings(const MODULE_SETTINGS & s) { settings = s; };
    int                 ParseSettings();

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; };
    bool                IsRunning() { return running; };

    const string      & GetStrError() const { return errorStr; };
    string              GetVersion() const;
    uint16_t            GetStartPosition() const;
    uint16_t            GetStopPosition() const;

private:
    static void *       Run(void *);
    void                RealRun();
    bool                PrepareNetwork();
    bool                FinalizeNetwork();
    void                AcceptConnection();

    mutable string      errorStr;
    STG_CONFIG_SETTINGS stgConfigSettings;
    pthread_t           thread;
    bool                running;
    bool                stopped;
    CONFIGPROTO         config;
    USERS *             users;
    ADMINS *            admins;
    TARIFFS *           tariffs;
    BASE_STORE *        store;
    MODULE_SETTINGS     settings;
    const SETTINGS *    stgSettings;

    std::list<ConnectionThread *> connections;

    int                 sd;
};
//-----------------------------------------------------------------------------
