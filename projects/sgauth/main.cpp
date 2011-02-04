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


#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "conffiles.h"
#endif

#include <string.h>
#include <vector>
#include <iostream>

#include "ia_auth_c.h"
#include "common.h"
#include "common_settings.h"
#include "web.h"

int mes;
char infoText[256];
char messageText[256];

const int winKOI = 0;

IA_CLIENT_PROT * clnp;
WEB * web = NULL;

using namespace std;

time_t stgTime;

//-----------------------------------------------------------------------------
class SETTINGS: public COMMON_SETTINGS
{
public:
                    SETTINGS();
    virtual         ~SETTINGS(){};
    virtual int     Reload(){ return 0; };
    void            SetConfFile(string confFile);
    virtual int     ReadSettings();

    virtual string  GetStrError() const;

    string          GetServerName() const;
    uint16_t        GetServerPort() const;
    uint16_t        GetLocalPort() const;

    string          GetLogin() const;
    string          GetPassword() const;

    bool            GetDaemon() const;
    bool            GetShowPid() const;
    bool            GetNoWeb() const;
    bool            GetReconnect() const;
    int             GetRefreshPeriod() const;
    uint32_t        GetListenWebIP() const;

    void            Print() const;

private:
    string          login;
    string          password;
    string          serverName;
    int             port;
    int             localPort;
    uint32_t        listenWebIP;
    int             refreshPeriod;

    bool            daemon;
    bool            noWeb;
    bool            reconnect;
    bool            showPid;

    string          confFile;
};
//-----------------------------------------------------------------------------
SETTINGS::SETTINGS()
    : port(0),
      localPort(0),
      listenWebIP(0),
      refreshPeriod(0),
      daemon(false),
      noWeb(false),
      reconnect(false),
      showPid(false)
{
confFile = "/etc/sgauth.conf";
}
//-----------------------------------------------------------------------------
void SETTINGS::SetConfFile(string confFile)
{
SETTINGS::confFile = confFile;
}
//-----------------------------------------------------------------------------
int SETTINGS::ReadSettings()
{
CONFIGFILE * cf;

cf = new CONFIGFILE(confFile);
string tmp;
int e = cf->Error();

if (e)
    {
    printf("Cannot read file.\n");
    delete cf;
    return -1;
    }

cf->ReadString("Login", &login, "/?--?--?*");
if (login == "/?--?--?*")
    {
    strError = "Parameter \'Login\' not found.";
    delete cf;
    return -1;
    }

cf->ReadString("Password", &password, "/?--?--?*");
if (login == "/?--?--?*")
    {
    strError = "Parameter \'Password\' not found.";
    delete cf;
    return -1;
    }

cf->ReadString("ServerName", &serverName, "?*?*?");
if (serverName == "?*?*?")
    {
    strError = "Parameter \'ServerName\' not found.";
    delete cf;
    return -1;
    }

cf->ReadString("ListenWebIP", &tmp, "127.0.0.1");
listenWebIP = inet_addr(tmp.c_str());
if (listenWebIP == INADDR_NONE)
    {
    strError = "Parameter \'ListenWebIP\' is not valid.";
    delete cf;
    return -1;
    }

cf->ReadString("ServerPort", &tmp, "5555");
if (ParseIntInRange(tmp, 1, 65535, &port))
    {
    strError = "Parameter \'ServerPort\' is not valid.";
    delete cf;
    return -1;
    }

cf->ReadString("LocalPort", &tmp, "0");
if (ParseIntInRange(tmp, 0, 65535, &localPort))
    {
    strError = "Parameter \'LocalPort\' is not valid.";
    delete cf;
    return -1;
    }

printf("LocalPort=%d\n", localPort);

cf->ReadString("RefreshPeriod", &tmp, "5");
if (ParseIntInRange(tmp, 1, 24*3600, &refreshPeriod))
    {
    strError = "Parameter \'RefreshPeriod\' is not valid.";
    delete cf;
    return -1;
    }

cf->ReadString("Reconnect", &tmp, "yes");
if (ParseYesNo(tmp, &reconnect))
    {
    strError = "Parameter \'Reconnect\' is not valid.";
    delete cf;
    return -1;
    }

cf->ReadString("Daemon", &tmp, "yes");
if (ParseYesNo(tmp, &daemon))
    {
    strError = "Parameter \'Daemon\' is not valid.";
    delete cf;
    return -1;
    }

cf->ReadString("ShowPid", &tmp, "no");
if (ParseYesNo(tmp, &showPid))
    {
    strError = "Parameter \'ShowPid\' is not valid.";
    delete cf;
    return -1;
    }

cf->ReadString("DisableWeb", &tmp, "no");
if (ParseYesNo(tmp, &noWeb))
    {
    strError = "Parameter \'DisableWeb\' is not valid.";
    delete cf;
    return -1;
    }

delete cf;
return 0;
}
//-----------------------------------------------------------------------------
string SETTINGS::GetStrError() const
{
return strError;
}
//-----------------------------------------------------------------------------
string SETTINGS::GetLogin() const
{
return login;
}
//-----------------------------------------------------------------------------
string SETTINGS::GetPassword() const
{
return password;
}
//-----------------------------------------------------------------------------
string SETTINGS::GetServerName() const
{
return serverName;
}
//-----------------------------------------------------------------------------
uint16_t SETTINGS::GetServerPort() const
{
return port;
}
//-----------------------------------------------------------------------------
uint16_t SETTINGS::GetLocalPort() const
{
return localPort;
}
//-----------------------------------------------------------------------------
int SETTINGS::GetRefreshPeriod() const
{
return refreshPeriod;
}
//-----------------------------------------------------------------------------
bool SETTINGS::GetDaemon() const
{
return daemon;
}
//-----------------------------------------------------------------------------
bool SETTINGS::GetNoWeb() const
{
return noWeb;
}
//-----------------------------------------------------------------------------
bool SETTINGS::GetShowPid() const
{
return showPid;
}
//-----------------------------------------------------------------------------
bool SETTINGS::GetReconnect() const
{
return reconnect;
}
//-----------------------------------------------------------------------------
uint32_t SETTINGS::GetListenWebIP() const
{
return listenWebIP;
}
//-----------------------------------------------------------------------------
void SETTINGS::Print() const
{
cout << "login = " << login << endl;
cout << "password = " << password << endl;
cout << "ip = " << serverName << endl;
cout << "port = " << port << endl;
cout << "localPort = " << localPort << endl;
cout << "listenWebIP = " << inet_ntostring(listenWebIP) << endl;
cout << "DisableWeb = " << noWeb << endl;
cout << "refreshPeriod = " << refreshPeriod << endl;
cout << "daemon = " << daemon << endl;
cout << "reconnect = " << reconnect << endl;
}
//-----------------------------------------------------------------------------
void Usage()
{
printf("sgauth <server> <port> <login> <password>\n"); //TODO change to correct
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
#ifndef WIN32
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
#endif
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
SETTINGS settings;

#ifndef WIN32
if (argc == 2)
#else
if(0)
#endif
    {
    settings.SetConfFile(argv[1]);
    }
else
    {
    /*if (argc != 5)
        {
        Usage();
        exit(1);
        }
    else
        {
        string serverName(argv[1]);
        port = strtol(argv[2], &endptr, 10);
        if (*endptr != 0)
            {
            printf("Invalid port!\n");
            exit(1);
            }
        login = argv[3];
        passwd = argv[4];
        }*/
    }

if (settings.ReadSettings())
    {
    printf("ReadSettingsError\n");
    printf("%s\n", settings.GetStrError().c_str());
    exit(-1);
    }
settings.Print();

#ifndef WIN32
if (settings.GetDaemon())
    {
    /*close(0);
    close(1);
    close(2);*/

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
#endif

clnp = new IA_CLIENT_PROT(settings.GetServerName(), settings.GetServerPort(), settings.GetLocalPort());

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
    #ifdef WIN32
    Sleep(200);
    #else
    usleep(200000);
    #endif

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
