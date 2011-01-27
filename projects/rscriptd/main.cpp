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
#include <set>

#include "common.h"
#include "stg_logger.h"
#include "script_executer.h"
#include "conffiles.h"
#include "listener.h"
#include "pidfile.h"
#include "version.h"

using namespace std;

#ifdef DEBUG
# define MAIN_DEBUG  1
# define NO_DAEMON    1
#endif

#define START_FILE "/._ST_ART_ED_"

static bool childExited = false;
set<pid_t> executersPid;
static pid_t stgChildPid;
volatile time_t stgTime = time(NULL);

class STG_STOPPER
{
public:
    STG_STOPPER() { nonstop = true; }
    bool GetStatus() const { return nonstop; };
    void Stop(const char * __file__, int __line__)
        {
        #ifdef NO_DAEMON
        printfd(__FILE__, "rscriptd stopped at %s:%d\n", __file__, __line__);
        #endif
        nonstop = false;
        }
private:
    bool nonstop;
};
//-----------------------------------------------------------------------------
STG_STOPPER nonstop;
//-----------------------------------------------------------------------------
void CatchPROF(int)
{
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
 *Parameters: sig_num - signal number
 *Description: INT signal handler
 *Returns: Nothing
 */
STG_LOGGER & WriteServLog = GetStgLogger();
WriteServLog("Shutting down... %d", sig);

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
    }
if (childPid == stgChildPid)
    {
    childExited = true;
    }
}
//-----------------------------------------------------------------------------
void SetSignalHandlers()
{
struct sigaction newsa, oldsa;
sigset_t sigmask;

sigemptyset(&sigmask);
sigaddset(&sigmask, SIGTERM);
newsa.sa_handler = CatchTERM;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGTERM, &newsa, &oldsa);

sigemptyset(&sigmask);
sigaddset(&sigmask, SIGUSR1);
newsa.sa_handler = CatchUSR1;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGUSR1, &newsa, &oldsa);

sigemptyset(&sigmask);
sigaddset(&sigmask, SIGINT);
newsa.sa_handler = CatchTERM;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGINT, &newsa, &oldsa);

sigemptyset(&sigmask);
sigaddset(&sigmask, SIGPIPE);
newsa.sa_handler = CatchPIPE;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGPIPE, &newsa, &oldsa);

sigemptyset(&sigmask);
sigaddset(&sigmask, SIGHUP);
newsa.sa_handler = CatchHUP;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGHUP, &newsa, &oldsa);

sigemptyset(&sigmask);
sigaddset(&sigmask, SIGCHLD);
newsa.sa_handler = CatchCHLD;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGCHLD, &newsa, &oldsa);
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
        Executer(msgKey, *msgID, executerPid, procName);
        return 1;

    default:    // Parent
        if (executersPid.empty())
            Executer(msgKey, *msgID, executerPid, NULL);
        executersPid.insert(executerPid);
    }
return 0;
}
//-----------------------------------------------------------------------------
#ifdef NO_DAEMON
int ForkAndWait(const string &)
#else
int ForkAndWait(const string & confDir)
#endif
{
#ifndef NO_DAEMON
stgChildPid = fork();
string startFile = confDir + START_FILE;
unlink(startFile.c_str());

switch (stgChildPid)
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
        for (int i = 0; i < 120 * 5; i++)
            {
            if (access(startFile.c_str(), F_OK) == 0)
                {
                //printf("Fork successfull. Exit.\n");
                unlink(startFile.c_str());
                exit(0);
                }

            if (childExited)
                {
                unlink(startFile.c_str());
                exit(1);
                }
            usleep(200000);
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
  - Config
  - Set signal nandlers
  - Fork and exit
 */

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

SetSignalHandlers();
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

#ifndef NO_DAEMON
string startFile(confDir + START_FILE);
creat(startFile.c_str(), S_IRUSR);
#endif

while (nonstop.GetStatus())
    {
    usleep(100000);
    }

listener->Stop();

WriteServLog("+++++++++++++++++++++++++++++++++++++++++++++");

int res = msgctl(msgID, IPC_RMID, NULL);
if (res)
    WriteServLog("Queue was not removed. id=%d", msgID);
else
    WriteServLog("Queue removed successfully.");

KillExecuters();

WriteServLog("rscriptd stopped successfully.");
WriteServLog("---------------------------------------------");

delete listener;
delete cfg;
return EXIT_SUCCESS;
}
//-----------------------------------------------------------------------------
