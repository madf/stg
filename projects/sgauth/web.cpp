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
 $Revision: 1.7 $
 $Date: 2010/03/15 12:58:17 $
 */

#include <libintl.h>

#include <csignal>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "stg/common.h"
#include "stg/ia.h"
#include "web.h"

extern WEB * web;
extern IA_CLIENT_PROT * clnp;

#define LISTEN_PORT (5580)

#include "css.h"

//---------------------------------------------------------------------------
#ifndef WIN32
void * RunWeb(void *)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

#else
unsigned long WINAPI RunWeb(void *)
{
#endif
while (1)
    web->Run();
return NULL;
}
//---------------------------------------------------------------------------
WEB::WEB()
    : m_res(0),
      m_listenSocket(0),
      m_outerSocket(0),
      m_refreshPeriod(0),
      m_listenWebAddr(0)
{
#ifdef WIN32
m_res = WSAStartup(MAKEWORD(2,0), &m_wsaData);
#endif

for (int i = 0; i < DIR_NUM; i++)
    m_dirName[i] = "-";

m_refreshPeriod = 5;

memset(&m_ls, 0, sizeof(m_ls));
}
//---------------------------------------------------------------------------
void WEB::Start()
{
#ifdef WIN32
unsigned long pt;
CreateThread(
    NULL,   // pointer to thread security attributes
    16384,  // initial thread stack size, in bytes
    RunWeb, // pointer to thread function
    NULL,   // argument for new thread
    0,      // CREATE_SUSPENDED, // creation flags
    &pt     // pointer to returned thread identifier
   );
#else
pthread_create(&m_thread, NULL, RunWeb, NULL);
#endif
}
//---------------------------------------------------------------------------
void WEB::PrepareNet()
{
m_listenSocket = socket(PF_INET, SOCK_STREAM, 0);

struct sockaddr_in listenAddr;
listenAddr.sin_family = AF_INET;
listenAddr.sin_port = htons(LISTEN_PORT);
listenAddr.sin_addr.s_addr = m_listenWebAddr;

#ifndef WIN32
int lng = 1;
if (0 != setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, &lng, 4))
    {
    printf("Setsockopt Fail\n");
    printf(">>> Error %s\n", strerror(errno));
    }
#else
//??? TODO
#endif


m_res = bind(m_listenSocket, reinterpret_cast<sockaddr*>(&listenAddr), sizeof(listenAddr));

if (m_res == -1)
    {
    printf("Bind failed.\n");
    exit(0);
    }

m_res = listen(m_listenSocket, 0);
if (m_res == -1)
    {
    printf("Listen failed.\n");
    exit(0);
    }
}
//---------------------------------------------------------------------------
void WEB::SetRefreshPagePeriod(int p)
{
m_refreshPeriod = p;
if (m_refreshPeriod <= 0 || m_refreshPeriod > 24*3600)
    m_refreshPeriod = 5;
}
//---------------------------------------------------------------------------
void WEB::SetListenAddr(uint32_t ip)
{
m_listenWebAddr = ip;
}
//---------------------------------------------------------------------------
void WEB::Run()
{
PrepareNet();
char recvBuffer[4096];
while (1)
    {
    struct sockaddr_in outerAddr;

    #ifndef WIN32
    socklen_t outerAddrLen = sizeof(outerAddr);
    #else
    int outerAddrLen = sizeof(outerAddr);
    #endif

    m_outerSocket = accept(m_listenSocket, reinterpret_cast<sockaddr*>(&outerAddr), &outerAddrLen);
    if (m_outerSocket == -1)
        {
        printf(">>> Error %s\n", strerror(errno));
        continue;
        }
    recv(m_outerSocket, recvBuffer, sizeof(recvBuffer), 0);

    if (strncmp(recvBuffer, "GET /sgauth.css", strlen("GET /sgauth.css")) == 0)
        {
        SendCSS();
        //printf("(1) recvBuffer=%s\n", recvBuffer);
        }
    else if (strncmp(recvBuffer, "GET /disconnect", strlen("GET /disconnect")) == 0)
        {
        clnp->Disconnect();
        Redirect("/");
        //printf("(2) recvBuffer=%s\n", recvBuffer);
        }
    else if (strncmp(recvBuffer, "GET /connect", strlen("GET /connect")) == 0)
        {
        clnp->Connect();
        Redirect("/");
        //printf("(3) recvBuffer=%s\n", recvBuffer);
        }
    else if (strncmp(recvBuffer, "GET /exit", strlen("GET /exit")) == 0)
        {
        Redirect("/");
        clnp->Disconnect();
        #ifdef WIN32
        Sleep(1000);
        #else
        struct timespec ts = {1, 0};
        nanosleep(&ts, NULL);
        #endif
        exit(0);
        }
    else
       {
       SendReply();
       //printf("(4) recvBuffer=%s\n", recvBuffer);
       }

    #ifdef WIN32
    closesocket(m_outerSocket);
    #else
    close(m_outerSocket);
    #endif
    }
}
//---------------------------------------------------------------------------
int WEB::Redirect(const char * url)
{
const char * redirect =
    "HTTP/1.0 200 OK\n"
    "Content-Type: text/html\n"
    "Connection: close"
    "\n\n"
    "<html>\n"
    "<head>\n"
    "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0;%s\">\n"
    "</head>\n"
    "<body>\n"
    "</body></html>\n\n";

char buff[2000];
sprintf(buff, redirect, url);
send(m_outerSocket, buff, strlen(buff), 0);

return 0;
}
//---------------------------------------------------------------------------
int WEB::SendReply()
{
int j, rowNum;

const char * replyHeader =
    "HTTP/1.0 200 OK\n"
    "Content-Type: text/html\n"
    "Connection: close"
    "\n\n"
    "<html>\n"
    "<head>\n"
    "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"%d\">\n"
    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\">\n"
    "<title>sgauth</title>\n"
    "<link rel=\"Stylesheet\" href=\"sgauth.css\">"
    "</head>\n"
    "<body>\n"
    "<H3>Stargazer</H3><p>\n";

const char * replyFooter = "</body></html>\n\n";

char replyHeaderBuffer[2000];
sprintf(replyHeaderBuffer, replyHeader, m_refreshPeriod);

send(m_outerSocket, replyHeaderBuffer, strlen(replyHeaderBuffer), 0);

char str[512];

int st = clnp->GetAuthorized();

sprintf(str, "<a href=\"connect\">%s</a><p>\n", gettext("Connect"));
m_res = send(m_outerSocket, str, strlen(str), 0);

sprintf(str, "<a href=\"disconnect\">%s</a><p>\n", gettext("Disconnect"));
m_res = send(m_outerSocket, str, strlen(str), 0);

sprintf(str, "<a href=\"/\">%s</a><p>\n", gettext("Refresh"));
m_res = send(m_outerSocket, str, strlen(str), 0);

sprintf(str, "<a href=\"exit\">%s</a><p>\n", gettext("Exit"));
m_res = send(m_outerSocket, str, strlen(str), 0);

sprintf(str, "<div id=\"%s\">%s</div><p>\n" , st ? "ConnectionStateOnline":"ConnectionStateOffline", st ? "Online":"Offline");
m_res = send(m_outerSocket, str, strlen(str), 0);

sprintf(str, "<div id=\"Cash\">%s: %.3f</div><p>\n" , gettext("Cash"), m_ls.cash / 1000.0);
m_res = send(m_outerSocket, str, strlen(str), 0);

sprintf(str, "<div id=\"Prepaid Traffic\">%s: %s</div><p>\n" ,
        gettext("PrepaidTraffic"),
        m_ls.freeMb[0] == 'C' ? m_ls.freeMb + 1 : m_ls.freeMb);
m_res = send(m_outerSocket, str, strlen(str), 0);

sprintf(str, "<TABLE id=\"TraffTable\">\n");
m_res = send(m_outerSocket, str, strlen(str), 0);
sprintf(str, "    <TR id=\"TraffTableCaptionRow\">\n");
m_res = send(m_outerSocket, str, strlen(str), 0);
sprintf(str, "       <TD id=\"TraffTableCaptionCellC\">&nbsp;</TD>\n");
m_res = send(m_outerSocket, str, strlen(str), 0);

rowNum = 0;
for (j = 0; j < DIR_NUM; j++)
    {
    if (m_dirName[j][0] == 0)
        continue;
    std::string s;
    KOIToWin(m_dirName[j], &s);// +++++++++ sigsegv ==========   TODO too long dir name crashes sgauth
    sprintf(str, "       <TD id=\"TraffTableCaptionCell%d\">%s</TD>\n", rowNum++, s.c_str());
    send(m_outerSocket, str, strlen(str), 0);
    }

sprintf(str,"    </TR>\n");
send(m_outerSocket, str, strlen(str), 0);

sprintf(str,"    <TR id=\"TraffTableUMRow\">\n");
send(m_outerSocket, str, strlen(str), 0);

sprintf(str,"        <TD id=\"TraffTableUMCellC\">%s</TD>\n", gettext("Month Upload"));
send(m_outerSocket, str, strlen(str), 0);

rowNum = 0;
for (j = 0; j < DIR_NUM; j++)
    {
    if (m_dirName[j][0] == 0)
        continue;
    sprintf(str,"        <TD id=\"TraffTableUMCell%d\">%s</TD>\n", rowNum++, IntToKMG(m_ls.mu[j], ST_F));
    m_res = send(m_outerSocket, str, strlen(str), 0);
    }

sprintf(str,"    </TR>\n");
m_res = send(m_outerSocket, str, strlen(str), 0);
sprintf(str,"    <TR id=\"TraffTableDMRow\">\n");
m_res = send(m_outerSocket, str, strlen(str), 0);
sprintf(str,"        <TD id=\"TraffTableDMCellC\">%s</TD>\n", gettext("Month Download"));
m_res = send(m_outerSocket, str, strlen(str), 0);

rowNum = 0;
for (j = 0; j < DIR_NUM; j++)
    {
    if (m_dirName[j][0] == 0)
        continue;
    sprintf(str,"        <TD id=\"TraffTableDMCell%d\">%s</TD>\n", rowNum++, IntToKMG(m_ls.md[j], ST_F));
    m_res = send(m_outerSocket, str, strlen(str), 0);
    }
sprintf(str,"    </TR>\n");
m_res = send(m_outerSocket, str, strlen(str), 0);


sprintf(str,"    <TR id=\"TraffTableUSRow\">\n");
m_res = send(m_outerSocket, str, strlen(str), 0);
sprintf(str,"        <TD id=\"TraffTableUSCellC\">%s</TD>\n", gettext("Session Upload"));
m_res = send(m_outerSocket, str, strlen(str), 0);

rowNum = 0;
for (j = 0; j < DIR_NUM; j++)
    {
    if (m_dirName[j][0] == 0)
        continue;
    sprintf(str,"        <TD id=\"TraffTableUSCell%d\">%s</TD>\n", rowNum++, IntToKMG(m_ls.su[j], ST_F));
    m_res = send(m_outerSocket, str, strlen(str), 0);
    }

sprintf(str,"    </TR>\n");
m_res = send(m_outerSocket, str, strlen(str), 0);
sprintf(str,"    <TR id=\"TraffTableDSRow\">\n");
m_res = send(m_outerSocket, str, strlen(str), 0);
sprintf(str,"        <TD id=\"TraffTableDSCellC\">%s</TD>\n", gettext("Session Download"));
m_res = send(m_outerSocket, str, strlen(str), 0);

for (j = 0; j < DIR_NUM; j++)
    {
    if (m_dirName[j][0] == 0)
        continue;
    sprintf(str,"        <TD id=\"TraffTableDSCell%d\">%s</TD>\n", j, IntToKMG(m_ls.sd[j], ST_F));
    m_res = send(m_outerSocket, str, strlen(str), 0);
    }

sprintf(str,"    </TR>\n");
m_res = send(m_outerSocket, str, strlen(str), 0);

sprintf(str,"</TABLE>\n");
m_res = send(m_outerSocket, str, strlen(str), 0);

rowNum = 0;
if (!m_messages.empty())
    {
    sprintf(str,"    <TABLE id=\"MessagesTable\">\n");
    m_res = send(m_outerSocket, str, strlen(str), 0);

    sprintf(str,"        <TR id=\"MessagesTableRowC\">\n");
    send(m_outerSocket, str, strlen(str), 0);
    sprintf(str,"            <TD>Date</TD>\n");
    send(m_outerSocket, str, strlen(str), 0);
    sprintf(str,"            <TD>Text</TD>\n");
    send(m_outerSocket, str, strlen(str), 0);
    sprintf(str,"        </TR>\n");
    send(m_outerSocket, str, strlen(str), 0);

    std::list<STG_MESSAGE>::reverse_iterator it;
    it = m_messages.rbegin();
    while (it != m_messages.rend())
        {
        sprintf(str,"        <TR id=\"MessagesTableRow%d\">\n", rowNum);
        send(m_outerSocket, str, strlen(str), 0);
        sprintf(str,"            <TD>%s</TD>\n", it->recvTime.c_str());
        send(m_outerSocket, str, strlen(str), 0);
        sprintf(str,"            <TD>%s</TD>\n", it->msg.c_str());
        send(m_outerSocket, str, strlen(str), 0);
        sprintf(str,"        </TR>\n");
        send(m_outerSocket, str, strlen(str), 0);
        ++it;
        ++rowNum;
        }

    sprintf(str,"   </TABLE>\n");
    m_res = send(m_outerSocket, str, strlen(str), 0);
    }

time_t t = time(NULL);
sprintf(str,"Обновлено: %s</b>" , ctime(&t));
m_res = send(m_outerSocket, str, strlen(str), 0);

send(m_outerSocket, replyFooter, strlen(replyFooter), 0);

return 0;
}
//---------------------------------------------------------------------------
int WEB::SendCSS()
{
const char * replyHeader =
    "HTTP/1.0 200 OK\n"
    "Content-Type: text/css\n"
    "Connection: close\n\n";

const char * replyFooter= "\n\n";

send(m_outerSocket, replyHeader, strlen(replyHeader), 0);
send(m_outerSocket, SGAuth::css, strlen(SGAuth::css), 0);
send(m_outerSocket, replyFooter, strlen(replyFooter), 0);

return 0;
}
//---------------------------------------------------------------------------
void WEB::SetDirName(const std::string & dn, int n)
{
web->m_dirName[n] =  dn;
}
//---------------------------------------------------------------------------
void WEB::AddMessage(const std::string & message, int type)
{
time_t t = time(NULL);
STG_MESSAGE m;

m.msg = message;
m.type = type;
m.recvTime = ctime(&t);

m_messages.push_back(m);

if (m_messages.size() > MAX_MESSAGES)
    m_messages.pop_front();

}
//---------------------------------------------------------------------------
void WEB::UpdateStat(const LOADSTAT & ls)
{
memcpy(&m_ls, &ls, sizeof(LOADSTAT));
}
//---------------------------------------------------------------------------

