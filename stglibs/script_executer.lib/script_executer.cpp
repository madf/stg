#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>

#include <cstring>
#include <cerrno>
#include <csignal>

#include "script_executer.h"

using namespace std;

#define MAX_SCRIPT_LEN  (1100)

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
nonstop = false;
}
//-----------------------------------------------------------------------------
int ScriptExec(const string & str)
{
if (str.length() >= MAX_SCRIPT_LEN)
    return -1;

int ret;
strncpy(sd.script, str.c_str(), MAX_SCRIPT_LEN);
sd.mtype = 1;
ret = msgsnd(msgid, (void *)&sd, MAX_SCRIPT_LEN, 0);
if (ret < 0)
    {
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
#ifdef LINUX
void Executer(int, int msgID, pid_t pid, char * procName)
#else
void Executer(int, int msgID, pid_t pid, char *)
#endif
{
msgid = msgID;
if (pid)
    return;
nonstop = true;

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
sigaddset(&sigmask, SIGHUP);
newsa.sa_handler = SIG_IGN;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGHUP, &newsa, &oldsa);

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
    ret = msgrcv(msgid, &sd, MAX_SCRIPT_LEN, 0, 0);

    if (ret < 0)
        {
        usleep(20000);
        continue;
        }
    int ret = system(sd.script);
    if (ret == -1)
        {
        // Fork failed
        }
    }
}
//-----------------------------------------------------------------------------


