#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "common.h"

using namespace std;

#define MAX_SCRIPT_LEN  (256)

static int msgid;
static bool nonstop;

//-----------------------------------------------------------------------------
struct SCRIPT_DATA
{
    long    mtype;
    char    script[MAX_SCRIPT_LEN];
} sd;
//-----------------------------------------------------------------------------
static void CatchUSR1Executer(int)
{
//printfd(__FILE__, "CatchUSR1Executer\n");
exit(0);
nonstop = false;
}
//-----------------------------------------------------------------------------
int ScriptExec(const string & str)
{
if (str.length() >= MAX_SCRIPT_LEN)
    return -1;

/*printfd(__FILE__, "2 Script schedule: %s\n", str.c_str());

if (access(str.c_str(), X_OK))
    return -1;*/

struct msgbuf * mbuf;
int ret;
strncpy(sd.script, str.c_str(), MAX_SCRIPT_LEN);
sd.mtype = 1;
mbuf = (struct msgbuf * )&sd;
ret = msgsnd(msgid, mbuf, MAX_SCRIPT_LEN, 0);
if (ret < 0)
    {
    printfd(__FILE__, "snd ERROR!\n");
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
void Executer(int msgKey, int msgID, pid_t pid, char * procName)
{
msgid = msgID;
if (pid)
    return;
nonstop = true;
//printfd(__FILE__, "close(pipeOut) %d pid=%d\n", pipeOut, getpid());

//printfd(__FILE__, "Executer started %d\n", getpid());
#ifdef LINUX
memset(procName, 0, strlen(procName));
strcpy(procName, "stg-exec");
#else
setproctitle("stg-exec");
#endif

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

sigemptyset(&sigmask);
sigaddset(&sigmask, SIGUSR1);
newsa.sa_handler = CatchUSR1Executer;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGUSR1, &newsa, &oldsa);

int ret;

SCRIPT_DATA sd;

while (nonstop)
    {
    sd.mtype = 1;
    //printfd(__FILE__, "Waiting for msgrcv... msgid=%d pid=%d\n", msgid, getpid());
    ret = msgrcv(msgid, &sd, MAX_SCRIPT_LEN, 0, 0);

    if (ret < 0)
        {
        usleep(20000);
        continue;
        }
    //printfd(__FILE__, "Script execute: %s\n", sd.script);
    system(sd.script);
    }
exit(0);
}
//-----------------------------------------------------------------------------


