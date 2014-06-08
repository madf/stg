#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "stg/scriptexecuter.h"

#define MAX_SCRIPT_LEN  (1100)

static int msgid;
static int nonstop;

//-----------------------------------------------------------------------------
struct SCRIPT_DATA
{
    long    mtype;
    char    script[MAX_SCRIPT_LEN];
} sd;
//-----------------------------------------------------------------------------
static void CatchUSR1Executer()
{
nonstop = 0;
}
//-----------------------------------------------------------------------------
int ScriptExec(const char * str)
{
if (strlen(str) >= MAX_SCRIPT_LEN)
    return -1;

strncpy(sd.script, str, MAX_SCRIPT_LEN);
sd.mtype = 1;
if (msgsnd(msgid, (void *)&sd, MAX_SCRIPT_LEN, 0) < 0)
    return -1;

return 0;
}
//-----------------------------------------------------------------------------
#if defined(LINUX) || defined(DARWIN)
void Executer(int msgID, pid_t pid, char * procName)
#else
void Executer(int msgID, pid_t pid)
#endif
{
struct SCRIPT_DATA sd;
struct sigaction newsa, oldsa;
sigset_t sigmask;

msgid = msgID;
if (pid)
    return;
nonstop = 1;

#if defined(LINUX) || defined(DARWIN)
memset(procName, 0, strlen(procName));
strcpy(procName, "stg-exec");
#else
setproctitle("stg-exec");
#endif

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

while (nonstop)
    {
    sd.mtype = 1;
    int ret = msgrcv(msgid, &sd, MAX_SCRIPT_LEN, 0, 0);

    if (ret < 0)
        {
        usleep(20000);
        continue;
        }
    ret = system(sd.script);
    if (ret == -1)
        {
        // Fork failed
        }
    }
}
//-----------------------------------------------------------------------------
