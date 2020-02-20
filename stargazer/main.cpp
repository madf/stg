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

using STG::SettingsImpl;
using STG::AdminsImpl;
using STG::TraffCounterImpl;
using STG::UsersImpl;
using STG::TariffsImpl;
using STG::ServicesImpl;
using STG::CorporationsImpl;
using STG::StoreLoader;

namespace
{
std::set<pid_t> executers;

void StartTimer();
int StartScriptExecuter(char* procName, int msgKey, int* msgID);
int ForkAndWait(const std::string& confDir);
void KillExecuters();

//-----------------------------------------------------------------------------
void StartTimer()
{
    auto& WriteServLog = STG::Logger::get();

    if (RunStgTimer())
    {
        WriteServLog("Cannot start timer. Fatal.");
        exit(1);
    }
    else
        WriteServLog("Timer thread started successfully.");
}
//-----------------------------------------------------------------------------
#if defined(LINUX) || defined(DARWIN)
int StartScriptExecuter(char* procName, int msgKey, int* msgID)
#else
int StartScriptExecuter(char*, int msgKey, int* msgID)
#endif
{
    auto& WriteServLog = STG::Logger::get();

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
                    msgctl(*msgID, IPC_RMID, NULL);
            }
            else
            {
                WriteServLog("Message queue created successfully. msgKey=%d msgID=%d", msgKey, *msgID);
                break;
            }
        }
    }

    const auto pid = fork();

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
int ForkAndWait(const std::string& confDir)
#else
int ForkAndWait(const std::string&)
#endif
{
#ifndef NO_DAEMON
    const auto pid = fork();
    const auto startFile = confDir + START_FILE;
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
    auto pid = executers.begin();
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
int main(int argc, char* argv[])
{
    int msgID = -11;

    STG::Logger::get().setFileName("/var/log/stargazer.log");

    if (getuid())
    {
        printf("You must be root. Exit.\n");
        return 1;
    }

    SettingsImpl settings(argc == 2 ? argv[1] : "");

    if (settings.ReadSettings())
    {
        auto& WriteServLog = STG::Logger::get();

        if (settings.GetLogFileName() != "")
            WriteServLog.setFileName(settings.GetLogFileName());

        WriteServLog("ReadSettings error. %s", settings.GetStrError().c_str());
        return -1;
    }

#ifndef NO_DAEMON
    const auto startFile = settings.GetConfDir() + START_FILE;
#endif

    if (ForkAndWait(settings.GetConfDir()) < 0)
    {
        STG::Logger::get()("Fork error!");
        return -1;
    }

    auto& WriteServLog = STG::Logger::get();
    WriteServLog.setFileName(settings.GetLogFileName());
    WriteServLog("Stg v. %s", SERVER_VERSION);

    for (size_t i = 0; i < settings.GetExecutersNum(); i++)
    {
        auto ret = StartScriptExecuter(argv[0], settings.GetExecMsgKey(), &msgID);
        if (ret < 0)
        {
            STG::Logger::get()("Start Script Executer error!");
            return -1;
        }
        if (ret == 1)
            return 0;
    }

    PIDFile pidFile(settings.GetPIDFileName());

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigaction(SIGHUP, &sa, NULL); // Apparently FreeBSD ignores SIGHUP by default when launched from rc.d at bot time.

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

    auto& loop = EVENT_LOOP_SINGLETON::GetInstance();

    StoreLoader storeLoader(settings);
    if (storeLoader.load())
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

    auto& store = storeLoader.get();
    WriteServLog("Storage plugin: %s. Loading successfull.", store.GetVersion().c_str());

    AdminsImpl admins(&store);
    TariffsImpl tariffs(&store);
    ServicesImpl services(&store);
    CorporationsImpl corps(&store);
    UsersImpl users(&settings, &store, &tariffs, services, admins.GetSysAdmin());
    TraffCounterImpl traffCnt(&users, settings.GetRulesFileName());
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
                SettingsImpl newSettings(settings);
                if (newSettings.ReadSettings())
                    WriteServLog("ReadSettings error. %s", newSettings.GetStrError().c_str());
                else
                    settings = newSettings;
                WriteServLog.setFileName(settings.GetLogFileName());
                traffCnt.Reload();
                manager.reload(settings);
                break;
            }
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

    manager.stop();

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
