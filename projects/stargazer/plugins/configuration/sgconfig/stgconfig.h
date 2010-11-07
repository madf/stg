#include <string>
#include <pthread.h>
#include "base_plugin.h"
#include "base_store.h"
#include "configproto.h"
//#include "user_ips.h"
//#include "../../../users.h"

using namespace std;

extern "C" BASE_PLUGIN * GetPlugin();

class STG_CONFIG;

//-----------------------------------------------------------------------------
/*template <typename varParamType>
class CHG_BEFORE_NOTIFIER: public PROPERTY_NOTIFIER_BASE<varParamType>
{
public:
    void Notify(const varParamType & oldValue, const varParamType & newValue)
        {
        auth->Unauthorize(user);
        }
    void SetUser(USER * u) { user = u; }
    void SetAuthorizaror(const AUTH_AO * a) { auth = a; }

private:
    USER * user;
    const AUTH_AO * auth;
};
//-----------------------------------------------------------------------------
template <typename varParamType>
class CHG_AFTER_NOTIFIER: public PROPERTY_NOTIFIER_BASE<varParamType>
{
public:
    void Notify(const varParamType & oldValue, const varParamType & newValue)
        {
        auth->UpdateUserAuthorization(user);
        }
    void SetUser(USER * u) { user = u; }
    void SetAuthorizaror(const AUTH_AO * a) { auth = a; }

private:
    USER * user;
    const AUTH_AO * auth;
};*/
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
class STG_CONFIG :public BASE_PLUGIN
{
public:
    STG_CONFIG();
    virtual ~STG_CONFIG(){};

    void                SetUsers(USERS * u);
    void                SetTariffs(TARIFFS * t);
    void                SetAdmins(ADMINS * a);
    void                SetStore(BASE_STORE * s);
    void                SetTraffcounter(TRAFFCOUNTER *){};
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
    static void *       Run(void *);
    mutable string      errorStr;
    STG_CONFIG_SETTINGS stgConfigSettings;
    pthread_t           thread;
    bool                nonstop;
    bool                isRunning;
    CONFIGPROTO         config;
    USERS *             users;
    ADMINS *            admins;
    TARIFFS *           tariffs;
    BASE_STORE *        store;
    MODULE_SETTINGS     settings;
    const SETTINGS *    stgSettings;
};
//-----------------------------------------------------------------------------


