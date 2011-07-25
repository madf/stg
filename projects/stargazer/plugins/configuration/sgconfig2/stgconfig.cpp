#include <cstdio>
#include <cunistd>
#include <csignal>
#include <functional>
#include <algorithm>

#include "stg/plugin_creator.h"
#include "stgconfig.h"
#include "../../../tariffs.h"
#include "../../../admins.h"
#include "../../../users.h"

PLUGIN_CREATOR<STG_CONFIG> stgc;

BASE_PLUGIN * GetPlugin()
{
return stgc.GetPlugin();
}

STG_CONFIG_SETTINGS::STG_CONFIG_SETTINGS()
    : port(0)
{
}

const string& STG_CONFIG_SETTINGS::GetStrError() const
{
return errorStr;
}

int STG_CONFIG_SETTINGS::ParseSettings(const MODULE_SETTINGS & s)
{
int p;
PARAM_VALUE pv;
vector<PARAM_VALUE>::const_iterator pvi;
///////////////////////////
pv.param = "Port";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
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

uint16_t STG_CONFIG_SETTINGS::GetPort()
{
return port;
}

STG_CONFIG::STG_CONFIG()
    : running(false),
      stopped(true)
{
}

string STG_CONFIG::GetVersion() const
{
return "Stg configurator v.2.00";
}

int STG_CONFIG::ParseSettings()
{
int ret = stgConfigSettings.ParseSettings(settings);
if (ret)
    errorStr = stgConfigSettings.GetStrError();
return ret;
}

int STG_CONFIG::Start()
{
if (running)
    return false;

if (PrepareNetwork())
    return true;

stopped = false;

config.SetPort(stgConfigSettings.GetPort());
config.SetAdmins(admins);
config.SetUsers(users);
config.SetTariffs(tariffs);
config.SetStgSettings(stgSettings);
config.SetStore(store);

if (config.Prepare())
    {
    errorStr = config.GetStrError();
    return true;
    }

if (pthread_create(&thread, NULL, Run, this))
    {
    errorStr = "Cannot create thread.";
    printfd(__FILE__, "Cannot create thread\n");
    return true;
    }

errorStr = "";
return false;
}

int STG_CONFIG::Stop()
{
if (!running)
    return false;

running = false;

config.Stop();

//5 seconds to thread stops itself
int i;
for (i = 0; i < 25 && !stopped; i++)
    {
    usleep(200000);
    }

//after 5 seconds waiting thread still running. now killing it
if (!stopped)
    {
    //TODO pthread_cancel()
    if (pthread_kill(thread, SIGINT))
        {
        errorStr = "Cannot kill thread.";
        printfd(__FILE__, "Cannot kill thread\n");
        return FinalizeNetwork();
        }
    printfd(__FILE__, "STG_CONFIG killed\n");
    }

return FinalizeNetwork();
}

void * STG_CONFIG::Run(void * d)
{
STG_CONFIG * stgConf = static_cast<STG_CONFIG *>(d);
stgConf->running = true;

stgConf->RealRun();

stgConf->stopped = true;
return NULL;
}

uint16_t STG_CONFIG::GetStartPosition() const
{
return 220;
}

uint16_t STG_CONFIG::GetStopPosition() const
{
return 220;
}

bool PrepareNetwork()
{
struct sockaddr_in local;

local.sin_family = AF_INET;
local.sin_port = htons(port);
local.sin_addr.s_addr = INADDR_ANY;

sd = socket(AF_INET, SOCK_STREAM, 0);
if (sd < 0)
    {
    errorStr = "Error creating socket: '";
    errorStr += strerror(errno);
    errorStr += "'";
    return true;
    }

if (bind(sd, static_cast<struct sockaddr *>(&local), sizeof(local)) < 0)
    {
    errorStr = "Error binding socket: '";
    errorStr += strerror(errno);
    errorStr += "'";
    return true;
    }

return false;
}

bool FinalizeNetwork()
{
if (close(sd) < 0)
    {
    errorStr = "Error closing socket: '";
    errorStr += strerror(errno);
    errorStr += "'";
    return true;
    }
return false;
}

void STG_CONFIG::RealRun()
{
if (listen(sd, 64) < 0)
    {
    errorStr = "Error listening socket: '";
    errorStr += strerror(errno);
    errorStr += "'";
    return;
    }

fd_set rfds;

FD_ZERO(&rfds);
FD_SET(sd, &rfds);

running = true;
while (running)
    {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000;

    int res = select(sd + 1, &rfds, NULL, NULL, &tv);

    if (res < 0)
        {
        // Error logging
        }
    else if (res == 0)
        {
        // Timeout
        }
    else
        {
        if (FD_ISSET(sd, &rfds))
            {
            AcceptConnection();
            }
        }

    // Reorder: right part is done
    std::list<ConnectionThread *>::iterator done(
            std::remove_if(
                connections.begin(),
                connections.end(),
                std::not1(std::mem_fun(&ConnectionThread::isDone))
            )
    );
    // Destruct done
    std::for_each(
            done,
            connections.end(),
            DeleteConnection());
    // Erase done
    std::erase(done, connections.end());

    }
stopped = true;
}

void STG_CONFIG::AcceptConnection()
{
struct sockaddr_in remoteAddr;
socklen_t len = sizeof(struct sockaddr_in);
int rsd = accept(sd, &remoteAddr, &len);

if (rsd < 0)
    {
    // Error logging
    }

connections.push_back(new ConnectionThread(this, rsd, remoteAddr, users, admins, tariffs, store, stgSettings));
}
