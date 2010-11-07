#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "xrconfig.h"
#include "../../../tariff2.h"
#include "../../../admins.h"
#include "../../../users.h"

class XR_CONFIG_CREATOR
{
private:
    XR_CONFIG * xrconfig;

public:
    XR_CONFIG_CREATOR()
        : xrconfig(new XR_CONFIG())
        {
        };
    ~XR_CONFIG_CREATOR()
        {
        delete xrconfig;
        };

    XR_CONFIG * GetPlugin()
        {
        return xrconfig;
        };
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
XRCONFIG_CREATOR xrc;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
XR_CONFIG_SETTINGS::XR_CONFIG_SETTINGS()
    : port(0)
{
}
//-----------------------------------------------------------------------------
const string & XR_CONFIG_SETTINGS::GetStrError() const
{
return errorStr;
}
//-----------------------------------------------------------------------------
int XR_CONFIG_SETTINGS::ParseIntInRange(const string & str, int min, int max, int * val)
{
if (strtoi2(str.c_str(), *val))
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
int XR_CONFIG_SETTINGS::ParseSettings(const MODULE_SETTINGS & s)
{
/*int p;
PARAM_VALUE pv;
vector<PARAM_VALUE>::const_iterator pvi;

pv.param = "Port";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end())
    {
    errorStr = "Parameter \'Port\' not found.";
    return -1;
    }

if (ParseIntInRange(pvi->value[0], 2, 65535, &p))
    {
    errorStr = "Cannot parse parameter \'Port\': " + errorStr;
    return -1;
    }
port = p;*/

return 0;
}
//-----------------------------------------------------------------------------
uint16_t XR_CONFIG_SETTINGS::GetPort()
{
return port;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BASE_PLUGIN * GetPlugin()
{
return xrc.GetPlugin();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const string XR_CONFIG::GetVersion() const
{
return "XR_configurator v.0.01";
}
//-----------------------------------------------------------------------------
XR_CONFIG::XR_CONFIG()
{
isRunning = false;
nonstop = false;
}
//-----------------------------------------------------------------------------
void XR_CONFIG::SetUsers(USERS * u)
{
users = u;
}
//-----------------------------------------------------------------------------
void XR_CONFIG::SetTariffs(TARIFFS * t)
{
tariffs = t;
}
//-----------------------------------------------------------------------------
void XR_CONFIG::SetAdmins(ADMINS * a)
{
admins = a;
}
//-----------------------------------------------------------------------------
void XR_CONFIG::SetStore(BASE_STORE * s)
{
store = s;
}
//-----------------------------------------------------------------------------
void XR_CONFIG::SetStgSettings(const SETTINGS * s)
{
stgSettings = s;
}
//-----------------------------------------------------------------------------
void XR_CONFIG::SetSettings(const MODULE_SETTINGS & s)
{
settings = s;
}
//-----------------------------------------------------------------------------
int XR_CONFIG::ParseSettings()
{
int ret = xrConfigSettings.ParseSettings(settings);
if (ret)
    errorStr = xrConfigSettings.GetStrError();
return ret;
}
//-----------------------------------------------------------------------------
const string & XR_CONFIG::GetStrError() const
{
return errorStr;
}
//-----------------------------------------------------------------------------
int XR_CONFIG::Start()
{
if (isRunning)
    return 0;

nonstop = true;

config.SetPort(xrConfigSettings.GetPort());
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
int XR_CONFIG::Stop()
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

    stgUsleep(200000);
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
    printfd(__FILE__, "XR_CONFIG killed\n");
    }

return 0;
}
//-----------------------------------------------------------------------------
bool XR_CONFIG::IsRunning()
{
return isRunning;
}
//-----------------------------------------------------------------------------
void * XR_CONFIG::Run(void * d)
{
XR_CONFIG * stgConf = (XR_CONFIG *)d;
stgConf->isRunning = true;

stgConf->config.Run(&stgConf->config);

stgConf->isRunning = false;
return NULL;
}
//-----------------------------------------------------------------------------
uint16_t XR_CONFIG::GetStartPosition() const
{
return 221;
}
//-----------------------------------------------------------------------------
uint16_t XR_CONFIG::GetStopPosition() const
{
return 221;
}
//-----------------------------------------------------------------------------
int XR_CONFIG::SetUserCash(const string & admLogin, const string & usrLogin, double cash) const
{
return 0;
}
//-----------------------------------------------------------------------------

