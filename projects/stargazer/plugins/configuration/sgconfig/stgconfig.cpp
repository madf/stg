/*
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

#include "stgconfig.h"

#include "stg/plugin_creator.h"
#include "stg/common.h"

#include <algorithm>
#include <csignal>

#include <unistd.h>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static PLUGIN_CREATOR<STG_CONFIG> stgc;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int STG_CONFIG_SETTINGS::ParseSettings(const MODULE_SETTINGS & s)
{
int p;
PARAM_VALUE pv;
std::vector<PARAM_VALUE>::const_iterator pvi;
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
port = static_cast<uint16_t>(p);

return 0;
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
STG_CONFIG::STG_CONFIG()
    : errorStr(),
      stgConfigSettings(),
      thread(),
      nonstop(false),
      isRunning(false),
      logger(GetPluginLogger(GetStgLogger(), "conf_sg")),
      config(logger),
      settings()
{
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
int STG_CONFIG::Start()
{
if (isRunning)
    return 0;

nonstop = true;

config.SetPort(stgConfigSettings.GetPort());

if (config.Prepare())
    {
    errorStr = config.GetStrError();
    return -1;
    }

if (pthread_create(&thread, NULL, Run, this))
    {
    errorStr = "Cannot create thread.";
    printfd(__FILE__, "Cannot create thread\n");
    logger("Cannot create thread.");
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
for (int i = 0; i < 25; i++)
    {
    if (!isRunning)
        break;

    struct timespec ts = {0, 200000000};
    nanosleep(&ts, NULL);
    }

if (isRunning)
    return -1;

return 0;
}
//-----------------------------------------------------------------------------
void * STG_CONFIG::Run(void * d)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

STG_CONFIG * stgConf = static_cast<STG_CONFIG *>(d);
stgConf->isRunning = true;

stgConf->config.Run();

stgConf->isRunning = false;
return NULL;
}
//-----------------------------------------------------------------------------
