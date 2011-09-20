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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

 /*
 $Revision: 1.19 $
 $Author: faust $
 $Date: 2010/09/10 06:37:45 $
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h> // creat
#include <unistd.h>

#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cerrno>
#include <cstring> // strerror
#include <set>

#include "stg/common.h"
#include "stg/logger.h"
#include "stg/scriptexecuter.h"
#include "stg/conffiles.h"
#include "stg/version.h"
#include "listener.h"
#include "pidfile.h"

using namespace std;

#ifdef DEBUG
# define MAIN_DEBUG  1
# define NO_DAEMON    1
#endif

#define START_FILE "/._ST_ART_ED_"

set<pid_t> executersPid;
volatile time_t stgTime = time(NULL);

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
int StartScriptExecuter(char * procName, int msgKey, int * msgID)
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
    case -1:    // Failure
        WriteServLog("Fork error!");
        return -1;

    case 0:     // Child
        //close(0);
        //close(1);
        //close(2);
        //setsid();
#ifdef LINUX
        Executer(*msgID, executerPid, procName);
#else
        Executer(*msgID, executerPid);
#endif
        return 1;

    default:    // Parent
        if (executersPid.empty())
#ifdef LINUX
            Executer(*msgID, executerPid, NULL);
#else
            Executer(*msgID, executerPid);
#endif
        executersPid.insert(executerPid);
    }
return 0;
}
//-----------------------------------------------------------------------------
void StopScriptExecuter(int msgID)
{
STG_LOGGER & WriteServLog = GetStgLogger();

for (int i = 0; i < 5; ++i)
    {
    struct msqid_ds data;
    if (msgctl(msgID, IPC_STAT, &data))
        {
        int e = errno;
        printfd(__FILE__, "StopScriptExecuter() - msgctl for IPC_STAT failed: '%s'\n", strerror(e));
        WriteServLog( "Failed to check queue emptiness: '%s'", strerror(e));
        break;
        }

    WriteServLog("Messages in queue: %d", data.msg_qnum);

    if (data.msg_qnum == 0)
        break;

    struct timespec ts = {1, 0};
    nanosleep(&ts, NULL);
    }

if (msgctl(msgID, IPC_RMID, NULL))
    {
    int e = errno;
    printfd(__FILE__, "StopScriptExecuter() - msgctl for IPC_STAT failed: '%s'\n", strerror(e));
    WriteServLog("Failed to remove queue: '%s'", strerror(e));
    }
else
    {
    WriteServLog("Queue removed successfully.");
    }

KillExecuters();
}
//-----------------------------------------------------------------------------
#ifdef NO_DAEMON
int ForkAndWait(const string &)
#else
int ForkAndWait(const string & confDir)
#endif
{
#ifndef NO_DAEMON
pid_t childPid = fork();

switch (childPid)
    {
    case -1:    // Failure
        return -1;
        break;

    case 0:     // Child
        //close(0);
        close(1);
        close(2);
        setsid();
        break;

    default:    // Parent
        exit(1);
        break;
    }
#endif
return 0;
}
//-----------------------------------------------------------------------------
int main(int argc, char * argv[])
{
CONFIGFILE * cfg = NULL;
LISTENER * listener = NULL;
int msgID = -11;
int execNum = 0;
int execMsgKey = 0;

string logFileName;
string confDir;
string password;
string onConnect;
string onDisconnect;
int port;
int userTimeout;

if (getuid())
    {
    printf("You must be root. Exit.\n");
    exit(1);
    }

if (argc == 2)
    cfg = new CONFIGFILE(argv[1]);
else
    cfg = new CONFIGFILE("/etc/rscriptd/rscriptd.conf");

if (cfg->Error())
    {
    STG_LOGGER & WriteServLog = GetStgLogger();
    WriteServLog.SetLogFileName("/var/log/rscriptd.log");
    WriteServLog("Error reading config file!");
    delete cfg;
    return EXIT_FAILURE;
    }

cfg->ReadString("LogFileName", &logFileName, "/var/log/rscriptd.log");
cfg->ReadInt("ExecutersNum", &execNum, 1);
cfg->ReadInt("ExecMsgKey", &execMsgKey, 5555);
cfg->ReadString("ConfigDir", &confDir, "/etc/rscriptd");
cfg->ReadString("Password", &password, "");
cfg->ReadInt("Port", &port, 5555);
cfg->ReadInt("UserTimeout", &userTimeout, 60);
cfg->ReadString("ScriptOnConnect", &onConnect, "/etc/rscriptd/OnConnect");
cfg->ReadString("ScriptOnDisconnect", &onDisconnect, "/etc/rscriptd/OnDisconnect");

if (ForkAndWait(confDir) < 0)
    {
    STG_LOGGER & WriteServLog = GetStgLogger();
    WriteServLog("Fork error!");
    delete cfg;
    return EXIT_FAILURE;
    }

STG_LOGGER & WriteServLog = GetStgLogger();
PIDFile pidFile("/var/run/rscriptd.pid");
WriteServLog.SetLogFileName(logFileName);
WriteServLog("rscriptd v. %s", SERVER_VERSION);

for (int i = 0; i < execNum; i++)
    {
    int ret = StartScriptExecuter(argv[0], execMsgKey, &msgID);
    if (ret < 0)
        {
        STG_LOGGER & WriteServLog = GetStgLogger();
        WriteServLog("Start Script Executer error!");
        delete cfg;
        return EXIT_FAILURE;
        }
    if (ret == 1)
        {
        delete cfg;
        return EXIT_SUCCESS;
        }
    }

listener = new LISTENER();
listener->SetPort(port);
listener->SetPassword(password);
listener->SetUserTimeout(userTimeout);
listener->SetScriptOnConnect(onConnect);
listener->SetScriptOnDisconnect(onDisconnect);

listener->Start();

WriteServLog("rscriptd started successfully.");
WriteServLog("+++++++++++++++++++++++++++++++++++++++++++++");

sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

while (true)
    {
    sigfillset(&signalSet);
    int sig = 0;
    printfd(__FILE__, "Before sigwait\n");
    sigwait(&signalSet, &sig);
    printfd(__FILE__, "After sigwait. Signal: %d\n", sig);
    bool stop = false;
    switch (sig)
        {
        case SIGTERM:
            stop = true;
            break;
        case SIGINT:
            stop = true;
            break;
        default:
            WriteServLog("Ignore signel %d", sig);
            break;
        }
    if (stop)
        break;
    }

listener->Stop();

WriteServLog("+++++++++++++++++++++++++++++++++++++++++++++");

StopScriptExecuter(msgID);

WriteServLog("rscriptd stopped successfully.");
WriteServLog("---------------------------------------------");

delete listener;
delete cfg;
return EXIT_SUCCESS;
}
//-----------------------------------------------------------------------------
