#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include <algorithm>

#include "stgconfig.h"
#include "tariffs.h"
#include "admins.h"
#include "users.h"

class STGCONFIG_CREATOR
{
private:
    STG_CONFIG * stgconfig;

public:
    STGCONFIG_CREATOR()
        : stgconfig(new STG_CONFIG())
        {
        };
    ~STGCONFIG_CREATOR()
        {
        delete stgconfig;
        };

    STG_CONFIG * GetPlugin()
        {
        return stgconfig;
        };
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
STGCONFIG_CREATOR stgc;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
STG_CONFIG_SETTINGS::STG_CONFIG_SETTINGS()
    : port(0)
{
}
//-----------------------------------------------------------------------------
const std::string & STG_CONFIG_SETTINGS::GetStrError() const
{
return errorStr;
}
//-----------------------------------------------------------------------------
int STG_CONFIG_SETTINGS::ParseIntInRange(const std::string & str, int min, int max, int * val)
{
if (str2x(str.c_str(), *val))
    {
    errorStr = "Incorrect value \'" + str + "\'.";
    return -1;
    }
if (*val < min || *val > max)
    {
    errorStr = "Value \'" + str + "\' out of range.";
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int STG_CONFIG_SETTINGS::ParseSettings(const MODULE_SETTINGS & s)
{
int p;
PARAM_VALUE pv;
vector<PARAM_VALUE>::const_iterator pvi;
///////////////////////////
pv.param = "Port";
pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end())
    {
    errorStr = "Parameter \'Port\' not found.";
    printfd(__FILE__, "Parameter 'Port' not found\n");
    return -1;
    }
if (ParseIntInRange(pvi->value[0], 2, 65535, &p))
    {
    errorStr = "Cannot parse parameter \'Port\': " + errorStr;
    printfd(__FILE__, "%s\n", errorStr.c_str());
    return -1;
    }
port = p;

return 0;
}
//-----------------------------------------------------------------------------
uint16_t STG_CONFIG_SETTINGS::GetPort() const
{
return port;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN * GetPlugin()
{
return stgc.GetPlugin();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const std::string STG_CONFIG::GetVersion() const
{
return "Stg configurator v.0.08";
}
//-----------------------------------------------------------------------------
STG_CONFIG::STG_CONFIG()
    : nonstop(false),
      isRunning(false),
      users(NULL),
      admins(NULL),
      tariffs(NULL),
      store(NULL),
      stgSettings(NULL)
{
}
//-----------------------------------------------------------------------------
void STG_CONFIG::SetUsers(USERS * u)
{
users = u;
}
//-----------------------------------------------------------------------------
void STG_CONFIG::SetTariffs(TARIFFS * t)
{
tariffs = t;
}
//-----------------------------------------------------------------------------
void STG_CONFIG::SetAdmins(ADMINS * a)
{
admins = a;
}
//-----------------------------------------------------------------------------
void STG_CONFIG::SetStore(BASE_STORE * s)
{
store = s;
}
//-----------------------------------------------------------------------------
void STG_CONFIG::SetStgSettings(const SETTINGS * s)
{
stgSettings = s;
}
//-----------------------------------------------------------------------------
void STG_CONFIG::SetSettings(const MODULE_SETTINGS & s)
{
settings = s;
}
//-----------------------------------------------------------------------------
int STG_CONFIG::ParseSettings()
{
int ret = stgConfigSettings.ParseSettings(settings);
if (ret)
    errorStr = stgConfigSettings.GetStrError();
return ret;
}
//-----------------------------------------------------------------------------
const std::string & STG_CONFIG::GetStrError() const
{
return errorStr;
}
//-----------------------------------------------------------------------------
int STG_CONFIG::Start()
{
if (isRunning)
    return 0;

nonstop = true;

config.SetPort(stgConfigSettings.GetPort());
config.SetAdmins(admins);
config.SetUsers(users);
config.SetTariffs(tariffs);
config.SetStgSettings(stgSettings);
config.SetStore(store);

if (config.Prepare())
    {
    errorStr = config.GetStrError();
    return -1;
    }

if (pthread_create(&thread, NULL, Run, this))
    {
    errorStr = "Cannot create thread.";
    printfd(__FILE__, "Cannot create thread\n");
    return -1;
    }
errorStr = "";
return 0;
}
//-----------------------------------------------------------------------------
int STG_CONFIG::Stop()
{
if (!isRunning)
    return 0;

config.Stop();

//5 seconds to thread stops itself
int i;
for (i = 0; i < 25; i++)
    {
    if (!isRunning)
        break;

    usleep(200000);
    }

//after 5 seconds waiting thread still running. now killing it
if (isRunning)
    {
    //TODO pthread_cancel()
    if (pthread_kill(thread, SIGINT))
        {
        errorStr = "Cannot kill thread.";
        printfd(__FILE__, "Cannot kill thread\n");
        return -1;
        }
    printfd(__FILE__, "STG_CONFIG killed\n");
    }

return 0;
}
//-----------------------------------------------------------------------------
bool STG_CONFIG::IsRunning()
{
return isRunning;
}
//-----------------------------------------------------------------------------
void * STG_CONFIG::Run(void * d)
{
STG_CONFIG * stgConf = (STG_CONFIG *)d;
stgConf->isRunning = true;

stgConf->config.Run(&stgConf->config);

stgConf->isRunning = false;
return NULL;
}
//-----------------------------------------------------------------------------
uint16_t STG_CONFIG::GetStartPosition() const
{
return 220;
}
//-----------------------------------------------------------------------------
uint16_t STG_CONFIG::GetStopPosition() const
{
return 220;
}
//-----------------------------------------------------------------------------


