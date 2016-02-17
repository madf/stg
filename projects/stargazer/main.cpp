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

#include "store_loader.h"
#include "plugin_mgr.h"
#include "plugin_runner.h"
#include "users_impl.h"
#include "admins_impl.h"
#include "tariffs_impl.h"
#include "services_impl.h"
#include "corps_impl.h"
#include "traffcounter_impl.h"
#include "settings_impl.h"
#include "pidfile.h"
#include "eventloop.h"
#include "stg_timer.h"

#include "stg/user.h"
#include "stg/common.h"
#include "stg/plugin.h"
#include "stg/logger.h"
#include "stg/scriptexecuter.h"
#include "stg/version.h"

#include <fstream>
#include <vector>
#include <set>
#include <csignal>
#include <cerrno>
#include <cstdio>
#include <cstdlib> // srandom, exit

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h> // S_IRUSR
#include <fcntl.h> // create

#ifdef DEBUG
    #define NO_DAEMON  (1)
#endif

#define START_FILE "/._ST_ART_ED_"

namespace
{
std::set<pid_t> executers;

void StartTimer();
int StartScriptExecuter(char * procName, int msgKey, int * msgID);
int ForkAndWait(const std::string & confDir);
void KillExecuters();

//-----------------------------------------------------------------------------
void StartTimer()
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
#if defined(LINUX) || defined(DARWIN)
int StartScriptExecuter(char * procName, int msgKey, int * msgID)
#else
int StartScriptExecuter(char *, int msgKey, int * msgID)
#endif
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

pid_t pid = fork();

switch (pid)
    {
    case -1:
        WriteServLog("Fork error!");
        return -1;

    case 0:
#if defined(LINUX) || defined(DARWIN)
        Executer(*msgID, pid, procName);
#else
        Executer(*msgID, pid);
#endif
        return 1;

    default:
        if (executers.empty()) {
#if defined(LINUX) || defined(DARWIN)
            Executer(*msgID, pid, NULL);
#else
            Executer(*msgID, pid);
#endif
        }
        executers.insert(pid);
    }
return 0;
}
//-----------------------------------------------------------------------------
#ifndef NO_DAEMON
int ForkAndWait(const std::string & confDir)
#else
int ForkAndWait(const std::string &)
#endif
{
#ifndef NO_DAEMON
pid_t pid = fork();
std::string startFile = confDir + START_FILE;
unlink(startFile.c_str());

switch (pid)
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
std::set<pid_t>::iterator pid(executers.begin());
while (pid != executers.end())
    {
    printfd(__FILE__, "KillExecuters pid=%d\n", *pid);
    kill(*pid, SIGUSR1);
    ++pid;
    }
}
//-----------------------------------------------------------------------------
} // namespace anonymous
//-----------------------------------------------------------------------------
int main(int argc, char * argv[])
{
int msgID = -11;

GetStgLogger().SetLogFileName("/var/log/stargazer.log");

if (getuid())
    {
    printf("You must be root. Exit.\n");
    return 1;
    }

SETTINGS_IMPL settings(argc == 2 ? argv[1] : "");

if (settings.ReadSettings())
    {
    STG_LOGGER & WriteServLog = GetStgLogger();

    if (settings.GetLogFileName() != "")
        WriteServLog.SetLogFileName(settings.GetLogFileName());

    WriteServLog("ReadSettings error. %s", settings.GetStrError().c_str());
    return -1;
    }

#ifndef NO_DAEMON
std::string startFile(settings.GetConfDir() + START_FILE);
#endif

if (ForkAndWait(settings.GetConfDir()) < 0)
    {
    STG_LOGGER & WriteServLog = GetStgLogger();
    WriteServLog("Fork error!");
    return -1;
    }

STG_LOGGER & WriteServLog = GetStgLogger();
WriteServLog.SetLogFileName(settings.GetLogFileName());
WriteServLog("Stg v. %s", SERVER_VERSION);

for (size_t i = 0; i < settings.GetExecutersNum(); i++)
    {
    int ret = StartScriptExecuter(argv[0], settings.GetExecMsgKey(), &msgID);
    if (ret < 0)
        {
        STG_LOGGER & WriteServLog = GetStgLogger();
        WriteServLog("Start Script Executer error!");
        return -1;
        }
    if (ret == 1)
        {
        // Stopping child
        return 0;
        }
    }

PIDFile pidFile(settings.GetPIDFileName());

sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

StartTimer();
WaitTimer();
if (!IsStgTimerRunning())
    {
    printfd(__FILE__, "Timer thread not started in 1 sec!\n");
    WriteServLog("Timer thread not started in 1 sec!");
    return -1;
    }

EVENT_LOOP & loop(EVENT_LOOP_SINGLETON::GetInstance());

STORE_LOADER storeLoader(settings);
if (storeLoader.Load())
    {
    printfd(__FILE__, "Storage plugin: '%s'\n", storeLoader.GetStrError().c_str());
    WriteServLog("Storage plugin: '%s'", storeLoader.GetStrError().c_str());
    return -1;
    }

if (loop.Start())
    {
    printfd(__FILE__, "Event loop not started.\n");
    WriteServLog("Event loop not started.");
    return -1;
    }

STORE & store(storeLoader.GetStore());
WriteServLog("Storage plugin: %s. Loading successfull.", store.GetVersion().c_str());

ADMINS_IMPL admins(&store);
TARIFFS_IMPL tariffs(&store);
SERVICES_IMPL services(&store);
CORPORATIONS_IMPL corps(&store);
USERS_IMPL users(&settings, &store, &tariffs, services, admins.GetSysAdmin());
TRAFFCOUNTER_IMPL traffCnt(&users, settings.GetRulesFileName());
traffCnt.SetMonitorDir(settings.GetMonitorDir());

if (users.Start())
    return -1;

WriteServLog("Users started successfully.");

if (traffCnt.Start())
    return -1;

WriteServLog("Traffcounter started successfully.");

STG::PluginManager manager(settings, store, admins, tariffs, services, corps, users, traffCnt);

srandom(static_cast<unsigned int>(stgTime));

WriteServLog("Stg started successfully.");
WriteServLog("+++++++++++++++++++++++++++++++++++++++++++++");

#ifndef NO_DAEMON
creat(startFile.c_str(), S_IRUSR);
#endif

bool running = true;
while (running)
    {
    sigfillset(&signalSet);
    int sig = 0;
    sigwait(&signalSet, &sig);
    int status;
    switch (sig)
        {
        case SIGHUP:
            {
            SETTINGS_IMPL newSettings(settings);
            if (newSettings.ReadSettings())
                {
                STG_LOGGER & WriteServLog = GetStgLogger();

                if (newSettings.GetLogFileName() != "")
                    WriteServLog.SetLogFileName(newSettings.GetLogFileName());

                WriteServLog("ReadSettings error. %s", newSettings.GetStrError().c_str());
                return -1;
                }
            settings = newSettings;
            traffCnt.Reload();
            manager.reload(settings);
            }
            break;
        case SIGTERM:
            running = false;
            break;
        case SIGINT:
            running = false;
            break;
        case SIGPIPE:
            WriteServLog("Broken pipe!");
            break;
        case SIGCHLD:
            executers.erase(waitpid(-1, &status, WNOHANG));
            if (executers.empty())
                running = false;
            break;
        default:
            WriteServLog("Ignore signal %d", sig);
            break;
        }
    }

WriteServLog("+++++++++++++++++++++++++++++++++++++++++++++");

if (loop.Stop())
    WriteServLog("Event loop not stopped.");

if (!traffCnt.Stop())
    WriteServLog("Traffcounter: Stop successfull.");

if (!users.Stop())
    WriteServLog("Users: Stop successfull.");

sleep(1);
int res = msgctl(msgID, IPC_RMID, NULL);
if (res)
    WriteServLog("Queue was not removed. id=%d", msgID);
else
    WriteServLog("Queue removed successfully.");

KillExecuters();

StopStgTimer();
WriteServLog("StgTimer: Stop successfull.");

WriteServLog("Stg stopped successfully.");
WriteServLog("---------------------------------------------");

return 0;
}
//-----------------------------------------------------------------------------
