#include <string>
#include <pthread.h>
#include "base_plugin.h"
//#include "common_settings.h"
#include "common.h"
//#include "configproto.h"

using namespace std;

extern "C" BASE_PLUGIN * GetPlugin();

class STG_CONFIG;

//-----------------------------------------------------------------------------
class XR_CONFIG_SETTINGS
{
public:
                    XR_CONFIG_SETTINGS();
    virtual         ~XR_CONFIG_SETTINGS(){};
    const string &  GetStrError() const;
    int             ParseSettings(const MODULE_SETTINGS & s);
    uint16_t        GetPort();

private:
    int     ParseIntInRange(const string & str, int min, int max, int * val);
    string  errorStr;
    int     port;
};
//-----------------------------------------------------------------------------
class XR_CONFIG :public BASE_PLUGIN
{
public:
    XR_CONFIG();
    virtual ~XR_CONFIG(){};

    void                SetUsers(USERS * u);
    void                SetTariffs(TARIFFS * t);
    void                SetAdmins(ADMINS * a);
    void                SetStore(BASE_STORE * s);
    void                SetTraffcounter(TRAFFCOUNTER * tc){};
    void                SetStgSettings(const SETTINGS * s);
    void                SetSettings(const MODULE_SETTINGS & s);
    int                 ParseSettings();

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; };
    bool                IsRunning();

    const string      & GetStrError() const;
    const string        GetVersion() const;
    uint16_t            GetStartPosition() const;
    uint16_t            GetStopPosition() const;

private:


    int                 SetUserCash(const string & admLogin, const string & usrLogin, double cash) const;

    static void *       Run(void *);
    mutable string      errorStr;
    XR_CONFIG_SETTINGS  xrConfigSettings;
    pthread_t           thread;
    bool                nonstop;
    bool                isRunning;

    //CONFIGPROTO         config;

    USERS *             users;
    ADMINS *            admins;
    TARIFFS *           tariffs;
    BASE_STORE *        store;
    MODULE_SETTINGS     settings;
    const SETTINGS *    stgSettings;
};
//-----------------------------------------------------------------------------


