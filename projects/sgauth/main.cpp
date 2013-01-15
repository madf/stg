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
 $Revision: 1.13 $
 $Date: 2010/04/14 09:01:29 $
 $Author: faust $
 */

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>

#include "stg/ia.h"
#include "stg/common.h"
#include "web.h"
#include "settings_impl.h"

int mes;
char infoText[256];
char messageText[256];

const int winKOI = 0;

IA_CLIENT_PROT * clnp;
WEB * web = NULL;

using namespace std;

time_t stgTime;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Usage()
{
printf("sgauth <path_to_config>\n");
}
//-----------------------------------------------------------------------------
void SetDirName(const vector<string> & dn, void *)
{
for (int j = 0; j < DIR_NUM; j++)
    {
    if (winKOI)
        {
        string dir;
        KOIToWin(dn[j], &dir);
        if (web)
            web->SetDirName(dir, j);
        }
    else
        {
        if (web)
            web->SetDirName(dn[j], j);
        }
    }
}
//-----------------------------------------------------------------------------
void StatUpdate(const LOADSTAT & ls, void *)
{
if (web)
    web->UpdateStat(ls);
}
//-----------------------------------------------------------------------------
void StatusChanged(int, void *)
{
}
//-----------------------------------------------------------------------------
void ShowMessage(const string & message, int i, int, int, void *)
{
if (web)
    web->AddMessage(message, i);
}
//-----------------------------------------------------------------------------
void ShowError(const string & message, int, void *)
{
if (web)
     web->AddMessage(message, 0);
}
//-----------------------------------------------------------------------------
void CatchUSR1(int)
{
if (clnp->GetAuthorized())
    {
    cout << "Connect" << endl;
    clnp->Connect();
    }
}
//-----------------------------------------------------------------------------
void CatchUSR2(int)
{
cout << "Disconnect" << endl;
clnp->Disconnect();
}
//-----------------------------------------------------------------------------
void CatchTERM(int)
{
cout << "Terminated" << endl;
clnp->Disconnect();
sleep(2);
exit(0);
}
//-----------------------------------------------------------------------------
static void SetSignalHandlers()
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
sigaddset(&sigmask, SIGINT);
newsa.sa_handler = CatchTERM;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGINT, &newsa, &oldsa);

sigemptyset(&sigmask);
sigaddset(&sigmask, SIGUSR1);
newsa.sa_handler = CatchUSR1;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGUSR1, &newsa, &oldsa);

sigemptyset(&sigmask);
sigaddset(&sigmask, SIGUSR2);
newsa.sa_handler = CatchUSR2;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGUSR2, &newsa, &oldsa);

return;
}
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
SETTINGS_IMPL settings;

if (argc == 2)
    {
    settings.SetConfFile(argv[1]);
    }
else
    {
    // Usage
    }

if (settings.ReadSettings())
    {
    printf("ReadSettingsError\n");
    printf("%s\n", settings.GetStrError().c_str());
    exit(-1);
    }
settings.Print();

if (settings.GetDaemon())
    {
    switch (fork())
        {
        case -1:
            exit(1);
            break;

        case 0:
            setsid();
            break;

        default:
            exit(0);
            break;
        }
    }

clnp = new IA_CLIENT_PROT(settings.GetServerName(), settings.GetServerPort(), settings.GetLocalName(), settings.GetLocalPort());

if (!settings.GetNoWeb())
    {
    web = new WEB();
    web->SetRefreshPagePeriod(settings.GetRefreshPeriod());
    web->SetListenAddr(settings.GetListenWebIP());
    web->Start();
    }

clnp->SetLogin(settings.GetLogin());
clnp->SetPassword(settings.GetPassword());

clnp->SetStatusChangedCb(StatusChanged, NULL);
clnp->SetInfoCb(ShowMessage, NULL);
clnp->SetErrorCb(ShowError, NULL);
clnp->SetDirNameCb(SetDirName, NULL);
clnp->SetStatChangedCb(StatUpdate, NULL);
clnp->SetReconnect(settings.GetReconnect());

clnp->Start();

SetSignalHandlers();

#ifdef LINUX
for (int i = 1; i < argc; i++)
    memset(argv[i], 0, strlen(argv[i]));

if(argc > 1)
    strcpy(argv[1], "Connecting...");
#endif

#ifdef FREEBSD
setproctitle("Connecting...");
#endif
clnp->Connect();

while (1)
    {
    struct timespec ts = {0, 200000000};
    nanosleep(&ts, NULL);

    char state[20];

    if (clnp->GetAuthorized())
        {
        if (settings.GetShowPid())
            sprintf(state, "On %d", getpid());
        else
            strcpy(state, "Online");
        }
    else
        {
        if (settings.GetShowPid())
            sprintf(state, "Off %d", getpid());
        else
            strcpy(state, "Offline");
        }

    #ifdef LINUX
    for (int i = 1; i < argc; i++)
        memset(argv[i], 0, strlen(argv[i]));
    if(argc > 1)
        strcpy(argv[1], state);
    #endif

    #ifdef FREEBSD
    setproctitle(state);
    #endif

    #ifdef FREEBSD_5
    setproctitle(state);
    #endif
    }

return 0;
}
//-----------------------------------------------------------------------------
