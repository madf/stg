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
uint32_t        eip;

#ifdef DEBUG
    #define MAIN_DEBUG (1)
    #define NO_DAEMON  (1)
#endif

#define START_FILE "/._ST_ART_ED_"

static bool needRulesReloading = false;
static bool childExited = false;
//static pid_t executerPid;
set<pid_t> executersPid;
static pid_t stgChildPid;


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
class STG_STOPPER
{
public:
    STG_STOPPER() : nonstop(true) {}
    bool    GetStatus() const { return nonstop; }
    #ifdef NO_DAEMON
    void    Stop(const char * __file__, int __line__)
    #else
    void    Stop(const char *, int)
    #endif
        {
        #ifdef NO_DAEMON
        printfd(__FILE__, "Stg stopped at %s:%d\n", __file__, __line__);
        #endif
        nonstop = false;
        }
private:
    bool nonstop;
};
//-----------------------------------------------------------------------------
STG_STOPPER nonstop;
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
void CatchUSR1(int)
{

}
//-----------------------------------------------------------------------------
void CatchTERM(int sig)
{
/*
 *Function Name:CatchINT
 *Parameters: sig_num - номер сигнала
 *Description: Обработчик сигнала INT
 *Returns: Ничего
 */
STG_LOGGER & WriteServLog = GetStgLogger();
WriteServLog("Shutting down... %d", sig);

//nonstop = false;
nonstop.Stop(__FILE__, __LINE__);

struct sigaction newsa, oldsa;
sigset_t sigmask;

sigemptyset(&sigmask);
sigaddset(&sigmask, SIGTERM);
newsa.sa_handler = SIG_IGN;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGTERM, &newsa, &oldsa);

sigemptyset(&sigmask);
sigaddset(&sigmask, SIGINT);
newsa.sa_handler = SIG_IGN;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGINT, &newsa, &oldsa);
}
//-----------------------------------------------------------------------------
void CatchPIPE(int)
{
STG_LOGGER & WriteServLog = GetStgLogger();
WriteServLog("Broken pipe!");
}
//-----------------------------------------------------------------------------
void CatchHUP(int)
{
needRulesReloading = true;
}
//-----------------------------------------------------------------------------
void CatchCHLD(int)
{
int status;
pid_t childPid;
childPid = waitpid(-1, &status, WNOHANG);

set<pid_t>::iterator pid;
pid = executersPid.find(childPid);
if (pid != executersPid.end())
    {
    executersPid.erase(pid);
    if (executersPid.empty() && nonstop.GetStatus())
        {
        nonstop.Stop(__FILE__, __LINE__);
        }
    }
if (childPid == stgChildPid)
    {
    childExited = true;
    }
}
/*//-----------------------------------------------------------------------------
void CatchSEGV(int, siginfo_t *, void *)
{
char fileName[50];
sprintf(fileName, "/tmp/stg_segv.%d", getpid());
FILE * f = fopen(fileName, "wt");
if (f)
    {
    fprintf(f, "\nSignal info:\n~~~~~~~~~~~~\n");
    fprintf(f, "numb:\t %d (%d)\n", sinfo->si_signo, sig);
    fprintf(f, "errn:\t %d\n", sinfo->si_errno);
    fprintf(f, "code:\t %d ", sinfo->si_code);

    switch (sinfo->si_code)
        {
        case SEGV_MAPERR:
            fprintf(f, "(SEGV_MAPERR - address not mapped to object)\n");
            break;

        case SEGV_ACCERR:
            fprintf(f, "(SEGV_ACCERR - invalid permissions for mapped object)\n");
            break;

        default:
            fprintf(f, "???\n");
        }

    fprintf(f, "addr:\t 0x%.8X\n",
        (unsigned int)sinfo->si_addr);

    Dl_info dlinfo;
    //asm("movl %eip, eip");
    if (dladdr((void*)CatchCHLD, &dlinfo))
        {
        fprintf(f, "SEGV point: %s %s\n", dlinfo.dli_fname, dlinfo.dli_sname);
        }
    else
        {
        fprintf(f, "Cannot find SEGV point\n");
        }

    fclose(f);
    }

struct sigaction segv_action, segv_action_old;

segv_action.sa_handler = SIG_DFL;
segv_action.sa_sigaction = NULL;
segv_action.sa_flags = SA_SIGINFO;
segv_action.sa_restorer = NULL;

sigaction(SIGSEGV, &segv_action, &segv_action_old);
}*/
//-----------------------------------------------------------------------------
static void SetSignalHandlers()
{
struct sigaction newsa, oldsa;
sigset_t sigmask;
///////
sigemptyset(&sigmask);
sigaddset(&sigmask, SIGTERM);
newsa.sa_handler = CatchTERM;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGTERM, &newsa, &oldsa);
///////
sigemptyset(&sigmask);
sigaddset(&sigmask, SIGUSR1);
newsa.sa_handler = CatchUSR1;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGUSR1, &newsa, &oldsa);
///////
sigemptyset(&sigmask);
sigaddset(&sigmask, SIGINT);
newsa.sa_handler = CatchTERM;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGINT, &newsa, &oldsa);
//////
sigemptyset(&sigmask);
sigaddset(&sigmask, SIGPIPE);
newsa.sa_handler = CatchPIPE;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGPIPE, &newsa, &oldsa);
//////
sigemptyset(&sigmask);
sigaddset(&sigmask, SIGHUP);
newsa.sa_handler = CatchHUP;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGHUP, &newsa, &oldsa);
//////
sigemptyset(&sigmask);
sigaddset(&sigmask, SIGCHLD);
newsa.sa_handler = CatchCHLD;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGCHLD, &newsa, &oldsa);

/*newsa.sa_handler = NULL;
newsa.sa_sigaction = CatchSEGV;
newsa.sa_flags = SA_SIGINFO;
newsa.sa_restorer = NULL;
sigaction(SIGSEGV, &newsa, &oldsa);*/

return;
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

            if (childExited)
                {
                unlink(startFile.c_str());
                exit(1);
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

/*
  Initialization order:
  - Logger
  - Stg timer
  - Settings
  - Plugins
  - Plugins settings
  - Read Admins
  - Read Tariffs
  - Read Users
  - Start Users
  - Start Traffcounter
  - Start Plugins
  - Start pinger
  - Set signal nandlers
  - Fork and exit
 * */

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
    //Loading modules
    if (modIter->Load())
        {
        WriteServLog("Error: %s",
                     modIter->GetStrError().c_str());
        goto exitLblNotStarted;
        }
    ++modIter;
    }

//Start section
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
        //printfd(__FILE__, "Error: %s\n", capRunner.GetStrError().c_str());
        goto exitLbl;
        }
    WriteServLog("Module: '%s'. Start successfull.", modIter->GetPlugin()->GetVersion().c_str());
    ++modIter;
    }
SetSignalHandlers();

srandom(stgTime);

/*
 * Note that an implementation in which nice returns the new nice value
 * can legitimately return -1.   To  reliably  detect  an  error,  set
 * errno to 0 before the call, and check its value when nice returns -1.
 *
 *
 * (c) man 2 nice
 */
/*errno = 0;
if (nice(-19) && errno) {
    printfd(__FILE__, "nice failed: '%s'\n", strerror(errno));
    WriteServLog("nice failed: '%s'", strerror(errno));
}*/

WriteServLog("Stg started successfully.");
WriteServLog("+++++++++++++++++++++++++++++++++++++++++++++");

#ifndef NO_DAEMON
creat(startFile.c_str(), S_IRUSR);
#endif

while (nonstop.GetStatus())
    {
    if (needRulesReloading)
        {
        needRulesReloading = false;
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
        }
    stgUsleep(100000);
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
        //printfd(__FILE__, "Error: %s\n", capRunner.GetStrError().c_str());
        //goto exitLbl;
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
    printfd(__FILE__, "Unloading module '%s'\n", name.c_str());
    if (modIter->Unload())
        {
        WriteServLog("Module \'%s\': Error: %s",
                     name.c_str(),
                     modIter->GetStrError().c_str());
        printfd(__FILE__, "Failed to unload module '%s'\n", name.c_str());
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
