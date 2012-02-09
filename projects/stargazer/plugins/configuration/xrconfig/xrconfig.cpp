#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "stg/plugin_creator.h"
#include "xrconfig.h"
#include "../../../tariff2.h"
#include "../../../admins.h"
#include "../../../users.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN_CREATOR<XR_CONFIG> xrc;
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
int XR_CONFIG_SETTINGS::ParseSettings(const MODULE_SETTINGS & s)
{

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
return 20;
}
//-----------------------------------------------------------------------------
uint16_t XR_CONFIG::GetStopPosition() const
{
return 20;
}
//-----------------------------------------------------------------------------
int XR_CONFIG::SetUserCash(const string & admLogin, const string & usrLogin, double cash) const
{
return 0;
}
//-----------------------------------------------------------------------------

