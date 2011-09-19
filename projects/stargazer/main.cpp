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

 /*
 $Revision: 1.124 $
 $Date: 2010/10/04 20:19:12 $
 $Author: faust $
 */

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h> // S_IRUSR
#include <fcntl.h> // create

#include <csignal>
#include <cerrno>
#include <cstdio>
#include <cstdlib> // srandom, exit
#include <fstream>
#include <vector>
#include <set>
#include <list>

#include "stg/user.h"
#include "stg/common.h"
#include "stg/plugin.h"
#include "stg/logger.h"
#include "stg/scriptexecuter.h"
#include "stg/conffiles.h"
#include "stg/version.h"
#include "stg/pinger.h"
#include "stg_timer.h"
#include "settings_impl.h"
#include "users_impl.h"
#include "admins_impl.h"
#include "tariffs_impl.h"
#include "services_impl.h"
#include "corps_impl.h"
#include "traffcounter_impl.h"
#include "plugin_runner.h"
#include "store_loader.h"
#include "pidfile.h"
#include "eventloop.h"

using namespace std;

#ifdef DEBUG
    #define MAIN_DEBUG (1)
    #define NO_DAEMON  (1)
#endif

#define START_FILE "/._ST_ART_ED_"

set<pid_t> executersPid;

//-----------------------------------------------------------------------------
bool StartModCmp(const PLUGIN_RUNNER & lhs, const PLUGIN_RUNNER & rhs)
{
return lhs.GetStartPosition() < rhs.GetStartPosition();
}
//-----------------------------------------------------------------------------
bool StopModCmp(const PLUGIN_RUNNER & lhs, const PLUGIN_RUNNER & rhs)
{
return lhs.GetStopPosition() > rhs.GetStopPosition();
}
//-----------------------------------------------------------------------------
static void StartTimer()
{
STG_LOGGER & WriteServLog = GetStgLogger();

if (RunStgTimer())
    {
    WriteServLog("Cannot start timer. Fatal.");
    //printfd(__FILE__, "Cannot start timer. Fatal.\n");
    exit(1);
    }
else
    {
    WriteServLog("Timer thread started successfully.");
    //printfd(__FILE__, "Timer thread started successfully.\n");
    }
}
//-----------------------------------------------------------------------------
int StartScriptExecuter(char * procName, int msgKey, int * msgID, SETTINGS_IMPL * settings)
{
STG_LOGGER & WriteServLog = GetStgLogger();

if (*msgID == -11)   // If msgID == -11 - first call. Create queue
    {
    for (int i = 0; i < 2; i++)
        {
        *msgID = msgget(msgKey, IPC_CREAT | IPC_EXCL | 0600);

        if (*msgID == -1)
            {
            *msgID = msgget(msgKey, 0);
            if (*msgID == -1)
                {
                WriteServLog("Message queue not created.");
                return -1;
                }
            else
                {
                msgctl(*msgID, IPC_RMID, NULL);
                }
            }
        else
            {
            WriteServLog("Message queue created successfully. msgKey=%d msgID=%d", msgKey, *msgID);
            break;
            }
        }
    }

pid_t executerPid = fork();

switch (executerPid)
    {
    case -1:
        WriteServLog("Fork error!");
        return -1;

    case 0:
        delete settings;
        Executer(*msgID, executerPid, procName);
        return 1;

    default:
        if (executersPid.empty()) {
            Executer(*msgID, executerPid, NULL);
        }
        executersPid.insert(executerPid);
    }
return 0;
}
//-----------------------------------------------------------------------------
#ifndef NO_DAEMON
int ForkAndWait(const string & confDir)
#else
int ForkAndWait(const string &)
#endif
{
#ifndef NO_DAEMON
stgChildPid = fork();
string startFile = confDir + START_FILE;
unlink(startFile.c_str());

switch (stgChildPid)
    {
    case -1:
        return -1;
        break;

    case 0:
        close(1);
        close(2);
        setsid();
        break;

    default:
        struct timespec ts = {0, 200000000};
        for (int i = 0; i < 120 * 5; i++)
            {
            if (access(startFile.c_str(), F_OK) == 0)
                {
                unlink(startFile.c_str());
                exit(0);
                }

            nanosleep(&ts, NULL);
            }
        unlink(startFile.c_str());
        exit(1);
        break;
    }
#endif
return 0;
}
//-----------------------------------------------------------------------------
void KillExecuters()
{
set<pid_t>::iterator pid;
pid = executersPid.begin();
while (pid != executersPid.end())
    {
    printfd(__FILE__, "KillExecuters pid=%d\n", *pid);
    kill(*pid, SIGUSR1);
    ++pid;
    }
}
//-----------------------------------------------------------------------------
int main(int argc, char * argv[])
{
SETTINGS_IMPL * settings = NULL;
STORE * dataStore = NULL;
TARIFFS_IMPL * tariffs = NULL;
ADMINS_IMPL * admins = NULL;
USERS_IMPL * users = NULL;
TRAFFCOUNTER_IMPL * traffCnt = NULL;
SERVICES_IMPL * services = NULL;
CORPORATIONS_IMPL * corps = NULL;
int msgID = -11;

    {
    STG_LOGGER & WriteServLog = GetStgLogger();
    WriteServLog.SetLogFileName("/var/log/stargazer.log");
    }

vector<MODULE_SETTINGS> modSettings;
list<PLUGIN_RUNNER> modules;

list<PLUGIN_RUNNER>::iterator modIter;

if (getuid())
    {
    printf("You must be root. Exit.\n");
    exit(1);
    }

if (argc == 2)
    settings = new SETTINGS_IMPL(argv[1]);
else
    settings = new SETTINGS_IMPL();

if (settings->ReadSettings())
    {
    STG_LOGGER & WriteServLog = GetStgLogger();

    if (settings->GetLogFileName() != "")
        WriteServLog.SetLogFileName(settings->GetLogFileName());

    WriteServLog("ReadSettings error. %s", settings->GetStrError().c_str());
    exit(1);
    }

#ifndef NO_DAEMON
string startFile(settings->GetConfDir() + START_FILE);
#endif

if (ForkAndWait(settings->GetConfDir()) < 0)
    {
    STG_LOGGER & WriteServLog = GetStgLogger();
    WriteServLog("Fork error!");
    exit(1);
    }

STG_LOGGER & WriteServLog = GetStgLogger();
WriteServLog.SetLogFileName(settings->GetLogFileName());
WriteServLog("Stg v. %s", SERVER_VERSION);

for (size_t i = 0; i < settings->GetExecutersNum(); i++)
    {
    int ret = StartScriptExecuter(argv[0], settings->GetExecMsgKey(), &msgID, settings);
    if (ret < 0)
        {
        STG_LOGGER & WriteServLog = GetStgLogger();
        WriteServLog("Start Script Executer error!");
        return 1;
        }
    if (ret == 1)
        {
        // Stopping child
        return 0;
        }
    }

PIDFile pidFile(settings->GetPIDFileName());

sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

StartTimer();
WaitTimer();
if (!IsStgTimerRunning())
    {
    printfd(__FILE__, "Timer thread not started in 1 sec!\n");
    WriteServLog("Timer thread not started in 1 sec!");
    }

EVENT_LOOP & loop(EVENT_LOOP_SINGLETON::GetInstance());

STORE_LOADER storeLoader(*settings);
if (storeLoader.Load())
    {
    WriteServLog("Storage plugin: '%s'", storeLoader.GetStrError().c_str());
    goto exitLblNotStarted;
    }

if (loop.Start())
    {
    WriteServLog("Event loop not started.");
    goto exitLblNotStarted;
    }

dataStore = storeLoader.GetStore();
WriteServLog("Storage plugin: %s. Loading successfull.", dataStore->GetVersion().c_str());

tariffs = new TARIFFS_IMPL(dataStore);
admins = new ADMINS_IMPL(dataStore);
users = new USERS_IMPL(settings, dataStore, tariffs, admins->GetSysAdmin());
traffCnt = new TRAFFCOUNTER_IMPL(users, settings->GetRulesFileName());
services = new SERVICES_IMPL(dataStore);
corps = new CORPORATIONS_IMPL(dataStore);
traffCnt->SetMonitorDir(settings->GetMonitorDir());

modSettings = settings->GetModulesSettings();

for (size_t i = 0; i < modSettings.size(); i++)
    {
    string modulePath = settings->GetModulesPath();
    modulePath += "/mod_";
    modulePath += modSettings[i].moduleName;
    modulePath += ".so";
    printfd(__FILE__, "Module: %s\n", modulePath.c_str());
    modules.push_back(
        PLUGIN_RUNNER(modulePath,
                      modSettings[i],
                      admins,
                      tariffs,
                      users,
                      services,
                      corps,
                      traffCnt,
                      dataStore,
                      settings)
        );
    }

modIter = modules.begin();

while (modIter != modules.end())
    {
    if (modIter->Load())
        {
        WriteServLog("Error: %s",
                     modIter->GetStrError().c_str());
        goto exitLblNotStarted;
        }
    ++modIter;
    }

if (users->Start())
    {
    goto exitLblNotStarted;
    }
WriteServLog("Users started successfully.");

if (traffCnt->Start())
    {
    goto exitLblNotStarted;
    }
WriteServLog("Traffcounter started successfully.");

//Sort by start order
modules.sort(StartModCmp);
modIter = modules.begin();

while (modIter != modules.end())
    {
    if (modIter->Start())
        {
        WriteServLog("Error: %s",
                     modIter->GetStrError().c_str());
        goto exitLbl;
        }
    WriteServLog("Module: '%s'. Start successfull.", modIter->GetPlugin()->GetVersion().c_str());
    ++modIter;
    }

srandom(stgTime);

WriteServLog("Stg started successfully.");
WriteServLog("+++++++++++++++++++++++++++++++++++++++++++++");

#ifndef NO_DAEMON
creat(startFile.c_str(), S_IRUSR);
#endif

while (true)
    {
    sigfillset(&signalSet);
    int sig = 0;
    sigwait(&signalSet, &sig);
    bool stop = false;
    int status;
    pid_t childPid;
    set<pid_t>::iterator it;
    switch (sig)
        {
        case SIGHUP:
            traffCnt->Reload();
            modIter = modules.begin();
            for (; modIter != modules.end(); ++modIter)
                {
                if (modIter->Reload())
                    {
                    WriteServLog("Error reloading %s ('%s')", modIter->GetPlugin()->GetVersion().c_str(),
                                                              modIter->GetStrError().c_str());
                    printfd(__FILE__, "Error reloading %s ('%s')\n", modIter->GetPlugin()->GetVersion().c_str(),
                                                                     modIter->GetStrError().c_str());
                    }
                }
            break;
        case SIGTERM:
            stop = true;
            break;
        case SIGINT:
            stop = true;
            break;
        case SIGPIPE:
            WriteServLog("Broken pipe!");
            break;
        case SIGCHLD:
            childPid = waitpid(-1, &status, WNOHANG);

            it = executersPid.find(childPid);
            if (it != executersPid.end())
                {
                executersPid.erase(it);
                if (executersPid.empty())
                    stop = true;
                }
            break;
        default:
            WriteServLog("Ignore signel %d", sig);
            break;
        }
    if (stop)
        break;
    }

exitLbl:

WriteServLog("+++++++++++++++++++++++++++++++++++++++++++++");

//Sort by start order
modules.sort(StopModCmp);
modIter = modules.begin();
while (modIter != modules.end())
    {
    std::string name = modIter->GetFileName();
    printfd(__FILE__, "Stopping module '%s'\n", name.c_str());
    if (modIter->Stop())
        {
        WriteServLog("Module \'%s\': Error: %s",
                     modIter->GetPlugin()->GetVersion().c_str(),
                     modIter->GetStrError().c_str());
        printfd(__FILE__, "Failed to stop module '%s'\n", name.c_str());
        }
    WriteServLog("Module: \'%s\'. Stop successfull.", modIter->GetPlugin()->GetVersion().c_str());
    ++modIter;
    }

if (loop.Stop())
    {
    WriteServLog("Event loop not stopped.");
    }

exitLblNotStarted:

modIter = modules.begin();
while (modIter != modules.end())
    {
    std::string name = modIter->GetFileName();
    if (modIter->IsRunning())
        {
        printfd(__FILE__, "Passing module '%s' `cause it's still running\n", name.c_str());
        }
    else
        {
        printfd(__FILE__, "Unloading module '%s'\n", name.c_str());
        if (modIter->Unload())
            {
            WriteServLog("Module \'%s\': Error: %s",
                         name.c_str(),
                         modIter->GetStrError().c_str());
            printfd(__FILE__, "Failed to unload module '%s'\n", name.c_str());
            }
        }
    ++modIter;
    }

if (traffCnt)
    {
    traffCnt->Stop();
    WriteServLog("Traffcounter: Stop successfull.");
    }

if (users)
    {
    users->Stop();
    WriteServLog("Users: Stop successfull.");
    }

sleep(1);
int res = msgctl(msgID, IPC_RMID, NULL);
if (res)
    WriteServLog("Queue was not removed. id=%d", msgID);
else
    WriteServLog("Queue removed successfully.");

KillExecuters();

StopStgTimer();
WriteServLog("StgTimer: Stop successfull.");

delete corps;
delete services;
delete traffCnt;
delete users;
delete admins;
delete tariffs;
delete settings;

WriteServLog("Stg stopped successfully.");
WriteServLog("---------------------------------------------");

return 0;
}
//-----------------------------------------------------------------------------
