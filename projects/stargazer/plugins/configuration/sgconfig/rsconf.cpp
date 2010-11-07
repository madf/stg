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

/*******************************************************************
*
*    DESCRIPTION: Файл с основными функциями для сетевого обмена данными
*    с менеджером клиентов. Прием, передача и шифрование сообщений.
*
*    AUTHOR: Boris Mikhailenko <stg34@stargazer.dp.ua>
*
*    $Revision: 1.24 $
*    $Date: 2010/10/04 20:24:54 $
*
*******************************************************************/

/*#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>*/

#include <cerrno>
#include <csignal>

#include "configproto.h"
#include "blowfish.h"

enum CONF_STATE
    {
    confHdr,
    confLogin,
    confLoginCipher,
    confData
    };

enum
    {
    ans_ok = 0,
    ans_err
    };

//-----------------------------------------------------------------------------
int CONFIGPROTO::Prepare()
{
list<string> ansList; //Сюда будет помещен ответ для менеджера клиентов
int res;
struct sockaddr_in listenAddr;

sigset_t sigmask, oldmask;
sigemptyset(&sigmask);
sigaddset(&sigmask, SIGINT);
sigaddset(&sigmask, SIGTERM);
sigaddset(&sigmask, SIGUSR1);
sigaddset(&sigmask, SIGHUP);
pthread_sigmask(SIG_BLOCK, &sigmask, &oldmask);

listenSocket = socket(PF_INET, SOCK_STREAM, 0);

if (listenSocket < 0)
    {
    errorStr = "Create NET_CONFIGURATOR socket failed.";
    return -1;
    }

listenAddr.sin_family = PF_INET;
listenAddr.sin_port = htons(port);
listenAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

int lng = 1;

if (0 != setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &lng, 4))
    {
    errorStr = "Setsockopt failed. " + string(strerror(errno));
    return -1;
    }

res = bind(listenSocket, (struct sockaddr*)&listenAddr, sizeof(listenAddr));

if (res == -1)
    {
    errorStr = "Bind admin socket failed";
    return -1;
    }

res = listen(listenSocket, 0);
if (res == -1)
    {
    errorStr = "Listen admin socket failed";
    return -1;
    }
outerAddrLen = sizeof(outerAddr);

/*if (0 != fcntl(listenSocket, F_SETFL, O_NONBLOCK))
    {
    errorStr = "fcntl error!";
    return -1;
    }*/

errorStr = "";
nonstop = true;
return 0;
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::Stop()
{
nonstop = false;
close(listenSocket);
//TODO: Idiotism
int                 sock;
struct sockaddr_in  addr;
socklen_t           addrLen;
addr.sin_family = PF_INET;
addr.sin_port = htons(port);
addr.sin_addr.s_addr = inet_addr("127.0.0.1");

addrLen = sizeof(outerAddr);
sock = socket(PF_INET, SOCK_STREAM, 0);
connect(sock, (sockaddr*)&addr, addrLen);
close(sock);
//Idiotism end
return 0;
}
//-----------------------------------------------------------------------------
// Функция общения с конфигуратором
void * CONFIGPROTO::Run(void * a)
{
/*
 * Function Name:ReciveSendConf
 * Parameters: void * a указатель на экземпляр класса CONFIGPROTO
 * Description: Эта функция обеспечивает сетевое взаимодействие
 *  с Менеджером Клиентов. В ее задачи входит: прием запросов по TCP/IP
 *  их расшифровка, передача принятых данных анализатору и отправка ответа назад.
 * Returns: возвращает NULL
 */

CONFIGPROTO * cp = (CONFIGPROTO*)a;
cp->state = confHdr;

while (cp->nonstop)
    {
    cp->state = confHdr;
    cp->outerSocket = accept(cp->listenSocket,
                             (struct sockaddr*)(&cp->outerAddr),
                             &cp->outerAddrLen);

    if (!cp->nonstop)
        {
        continue;
        }

    if (cp->outerSocket == -1)
        {
        printfd(__FILE__, "accept failed\n");
        usleep(100000);
        continue;
        }

    cp->adminIP = *(unsigned int*)&(cp->outerAddr.sin_addr);

    /* TODO
    if (!cp->hostAllow->HostAllowed(cp->adminIP))
        {
        close(outerSocket);
        continue;
        }*/

    printfd(__FILE__, "Connection accepted from %s\n", inet_ntostring(cp->outerAddr.sin_addr.s_addr).c_str());

    if (cp->state == confHdr)
        {
        if (cp->RecvHdr(cp->outerSocket) < 0)
            {
            close(cp->outerSocket);
            continue;
            }
        if (cp->state == confLogin)
            {
            if (cp->SendHdrAnswer(cp->outerSocket, ans_ok) < 0)
                {
                close(cp->outerSocket);
                continue;
                }

            if (cp->RecvLogin(cp->outerSocket) < 0)
                {
                close(cp->outerSocket);
                continue;
                }

            if (cp->state == confLoginCipher)
                {
                if (cp->SendLoginAnswer(cp->outerSocket, ans_ok) < 0)
                    {
                    close(cp->outerSocket);
                    continue;
                    }
                if (cp->RecvLoginS(cp->outerSocket) < 0)
                    {
                    close(cp->outerSocket);
                    continue;
                    }

                if (cp->state == confData)
                    {
                    if (cp->SendLoginSAnswer(cp->outerSocket, ans_ok) < 0)
                        {
                        close(cp->outerSocket);
                        continue;
                        }
                    if (cp->RecvData(cp->outerSocket) < 0)
                        {
                        close(cp->outerSocket);
                        continue;
                        }
                    cp->state = confHdr;
                    }
                else
                    {
                    if (cp->SendLoginSAnswer(cp->outerSocket, ans_err) < 0)
                        {
                        close(cp->outerSocket);
                        continue;
                        }
                    cp->WriteLogAccessFailed(cp->adminIP);
                    }
                }
            else
                {
                cp->WriteLogAccessFailed(cp->adminIP);
                }
            }
        else
            {
            cp->WriteLogAccessFailed(cp->adminIP);
            if (cp->SendHdrAnswer(cp->outerSocket, ans_err) < 0)
                {
                close(cp->outerSocket);
                continue;
                }
            }
        }
    else
        {
        cp->WriteLogAccessFailed(cp->adminIP);
        }
    close(cp->outerSocket);
    }

return NULL;
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::RecvHdr(int sock)
{
char buf[sizeof(STG_HEADER)];
memset(buf, 0, sizeof(STG_HEADER));
int ret;
int stgHdrLen = strlen(STG_HEADER);
for (int i = 0; i < stgHdrLen; i++)
    {
    ret = recv(sock, &buf[i], 1, 0);
    if (ret <= 0)
        {
        state = confHdr;
        return -1;
        }
    }

if (0 == strncmp(buf, STG_HEADER, strlen(STG_HEADER)))
    {
    state = confLogin;
    return 0;
    }
else
    {
    SendError("Bad request");
    }

state = confHdr;
return -1;
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::SendHdrAnswer(int sock, int err)
{
int ret;

if (err)
    {
    ret = send(sock, ERR_HEADER, sizeof(ERR_HEADER)-1, 0);
    if (ret < 0)
        {
        WriteServLog("send ERR_HEADER error in SendHdrAnswer.");
        return -1;
        }
    }
else
    {
    ret = send(sock, OK_HEADER, sizeof(OK_HEADER)-1, 0);
    if (ret < 0)
        {
        WriteServLog("send OK_HEADER error in SendHdrAnswer.");
        return -1;
        }
    }

return 0;
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::RecvLogin(int sock)
{
char login[ADM_LOGIN_LEN+1];
int ret;

memset(login, 0, ADM_LOGIN_LEN + 1);

//printfd(__FILE__, "RecvLogin\n");

/*for (int i = 0; i < ADM_LOGIN_LEN; i++)
    {
    ret = recv(sock, &login[i], 1, 0);

    if (ret <= 0)
        {
        close(sock);
        state = confHdr;
        return ENODATA;
        }
    }*/

ret = recv(sock, login, ADM_LOGIN_LEN, 0);

if (ret < 0)
    {
    // Error in network
    close(sock);
    state = confHdr;
    return ENODATA;
    }

if (ret < ADM_LOGIN_LEN)
    {
    // Error in protocol
    close(sock);
    state = confHdr;
    return ENODATA;
    }

if (admins->FindAdmin(login, &currAdmin))
    {
    // Admin not found
    close(sock);
    state = confHdr;
    return ENODATA;
    }
currAdmin.SetAdminIP(adminIP);
adminLogin = login;
state = confLoginCipher;
return 0;
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::SendLoginAnswer(int sock, int)
{
int ret;

ret = send(sock, OK_LOGIN, sizeof(OK_LOGIN)-1, 0);
if (ret < 0)
    {
    WriteServLog("Send OK_LOGIN error in SendLoginAnswer.");
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::RecvLoginS(int sock)
{
char loginS[ADM_LOGIN_LEN + 1];
char login[ADM_LOGIN_LEN + 1];
int ret;
BLOWFISH_CTX ctx;
memset(loginS, 0, ADM_LOGIN_LEN + 1);

//printfd(__FILE__, "RecvLoginS\n");

/*for (int i = 0; i < ADM_LOGIN_LEN; i++)
    {
    ret = recv(sock, &loginS[i], 1, 0);

    if (ret <= 0)
        {
        //printfd(__FILE__, "RecvLoginS close\n");
        close(sock);
        state = confHdr;
        return ENODATA;
        }
    }*/

int total = 0;

while (total < ADM_LOGIN_LEN)
    {
    ret = recv(sock, &loginS[total], ADM_LOGIN_LEN - total, 0);

    if (ret < 0)
        {
        // Network error
        printfd(__FILE__, "recv error: '%s'\n", strerror(errno));
        close(sock);
        state = confHdr;
        return ENODATA;
        }

    total += ret;
    }

// TODO: implement select on socket
/*if (total < ADM_LOGIN_LEN)
    {
    // Protocol error
    printfd(__FILE__, "Protocol error. Need %d bytes of cryptologin. Got %d bytes.\n", ADM_LOGIN_LEN, ret);
    close(sock);
    state = confHdr;
    return ENODATA;
    }*/

if (currAdmin.GetLogin() == "")
    {
    state = confHdr;
    return ENODATA;
    }

EnDecodeInit(currAdmin.GetPassword().c_str(), ADM_PASSWD_LEN, &ctx);

for (int i = 0; i < ADM_LOGIN_LEN/8; i++)
    {
    DecodeString(login + i*8, loginS + i*8, &ctx);
    }

if (currAdmin == admins->GetNoAdmin())
    {
    // If there are no admins registered in the system - give access with any password
    state = confData;
    return 0;
    }

if (strncmp(currAdmin.GetLogin().c_str(), login, ADM_LOGIN_LEN) != 0)
    {
    state = confHdr;
    return ENODATA;
    }

state = confData;
return 0;
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::SendLoginSAnswer(int sock, int err)
{
int ret;

if (err)
    {
    ret = send(sock, ERR_LOGINS, sizeof(ERR_LOGINS)-1, 0);
    if (ret < 0)
        {
        WriteServLog("send ERR_LOGIN error in SendLoginAnswer.");
        return -1;
        }
    }
else
    {
    ret = send(sock, OK_LOGINS, sizeof(OK_LOGINS)-1, 0);
    if (ret < 0)
        {
        WriteServLog("send OK_LOGINS error in SendLoginSAnswer.");
        return -1;
        }
    }
return 0;
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::RecvData(int sock)
{
//printfd(__FILE__, "RecvData\n");
//int n = 0;
int ret;
char bufferS[8];
char buffer[9];

buffer[8] = 0;

requestList.clear();
BLOWFISH_CTX ctx;

EnDecodeInit(currAdmin.GetPassword().c_str(), ADM_PASSWD_LEN, &ctx);

while (1)
    {
    /*ret = recv(sock, &bufferS[n++], 1, 0);
    if (ret <= 0)
        {
        //printfd(__FILE__, "RecvData close\n");
        close(sock);
        return 0;
        }*/
    int total = 0;
    bool done = false;
    while (total < 8)
        {
        ret = recv(sock, &bufferS[total], 8 - total, 0);
        if (ret < 0)
            {
            // Network error
            close(sock);
            return 0;
            }

        if (ret < 8)
            {
            if (memchr(buffer, 0, ret) != NULL)
                {
                done = true;
                break;
                }
            }

        total += ret;
        }

    DecodeString(buffer, bufferS, &ctx);
    requestList.push_back(std::string(buffer, total));

    if (done || memchr(buffer, 0, total) != NULL)
        {
        // Конец посылки
        if (ParseCommand())
            {
            SendError("Bad command");
            }
        return SendDataAnswer(sock);
        }

    /*if (n == 8)
        {
        n = 0;
        DecodeString(buffer, bufferS, &ctx);
        requestList.push_back(std::string(buffer, 8));
        for (int j = 0; j < 8; j++)
            {
            if (buffer[j] == 0)
                {
                // Конец посылки
                if (ParseCommand())
                    {
                    SendError("Bad command");
                    }
                return SendDataAnswer(sock);
                }
            }
        }*/
    }
return 0;
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::SendDataAnswer(int sock)
{
list<string>::iterator li;
li = answerList.begin();

BLOWFISH_CTX ctx;

char buff[8];
char buffS[8];
int n = 0;
int k = 0;
int ret = 0;

EnDecodeInit(currAdmin.GetPassword().c_str(), ADM_PASSWD_LEN, &ctx);

while (li != answerList.end())
    {
    while ((*li).c_str()[k])
        {
        buff[n%8] = (*li).c_str()[k];
        n++;
        k++;

        if (n%8 == 0)
            {
            EncodeString(buffS, buff, &ctx);
            ret = send(sock, buffS, 8, 0);
            if (ret < 0)
                {
                return -1;
                }
            }
        }
    k = 0;// new node
    li++;
    }

if (answerList.empty()) {
    return 0;
}

buff[n%8] = 0;
EncodeString(buffS, buff, &ctx);

answerList.clear();

return send(sock, buffS, 8, 0);
}
//-----------------------------------------------------------------------------
void CONFIGPROTO::SendError(const char * text)
{
char s[255];
answerList.clear();
sprintf(s, "<Error value=\"%s\"/>", text);
answerList.push_back(s);
}
//-----------------------------------------------------------------------------
void CONFIGPROTO::WriteLogAccessFailed(uint32_t ip)
{
WriteServLog("Admin's connect failed. IP %s", inet_ntostring(ip).c_str());
}
//-----------------------------------------------------------------------------



