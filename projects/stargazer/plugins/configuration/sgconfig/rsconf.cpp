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

#include <unistd.h> // close

#include <cerrno>
#include <csignal>
#include <cstdio> // snprintf

#include "stg/blowfish.h"
#include "configproto.h"

#ifndef ENODATA
// FreeBSD 4.* - suxx
#define ENODATA -1
#endif

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
std::list<std::string> ansList; //Сюда будет помещен ответ для менеджера клиентов
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
    logger("Cannot create a socket: %s", strerror(errno));
    return -1;
    }

listenAddr.sin_family = PF_INET;
listenAddr.sin_port = htons(port);
listenAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

int lng = 1;

if (0 != setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &lng, 4))
    {
    errorStr = "Setsockopt failed. " + std::string(strerror(errno));
    logger("setsockopt error: %s", strerror(errno));
    return -1;
    }

res = bind(listenSocket, (struct sockaddr*)&listenAddr, sizeof(listenAddr));

if (res == -1)
    {
    errorStr = "Bind admin socket failed";
    logger("Cannot bind the socket: %s", strerror(errno));
    return -1;
    }

res = listen(listenSocket, 0);
if (res == -1)
    {
    errorStr = "Listen admin socket failed";
    logger("Cannot listen the socket: %s", strerror(errno));
    return -1;
    }

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

addrLen = sizeof(addr);
sock = socket(PF_INET, SOCK_STREAM, 0);
connect(sock, (sockaddr*)&addr, addrLen);
close(sock);
//Idiotism end
return 0;
}
//-----------------------------------------------------------------------------
void CONFIGPROTO::Run()
{
state = confHdr;

while (nonstop)
    {
    state = confHdr;
    struct sockaddr_in outerAddr;
    socklen_t outerAddrLen(sizeof(outerAddr));
    int outerSocket = accept(listenSocket,
                             (struct sockaddr*)(&outerAddr),
                             &outerAddrLen);

    if (!nonstop)
        {
        break;
        }

    if (outerSocket < 0)
        {
	logger("accept error: %s", strerror(errno));
        printfd(__FILE__, "accept failed\n");
        continue;
        }

    adminIP = *(unsigned int*)&(outerAddr.sin_addr);

    if (state == confHdr)
        {
        if (RecvHdr(outerSocket) < 0)
            {
            close(outerSocket);
            continue;
            }
        if (state == confLogin)
            {
            if (SendHdrAnswer(outerSocket, ans_ok) < 0)
                {
                close(outerSocket);
                continue;
                }
            if (RecvLogin(outerSocket) < 0)
                {
                close(outerSocket);
                continue;
                }
            if (state == confLoginCipher)
                {
                if (SendLoginAnswer(outerSocket) < 0)
                    {
                    close(outerSocket);
                    continue;
                    }
                if (RecvLoginS(outerSocket) < 0)
                    {
                    close(outerSocket);
                    continue;
                    }
                if (state == confData)
                    {
                    if (SendLoginSAnswer(outerSocket, ans_ok) < 0)
                        {
                        close(outerSocket);
                        continue;
                        }
                    if (RecvData(outerSocket) < 0)
                        {
                        close(outerSocket);
                        continue;
                        }
                    state = confHdr;
                    }
                else
                    {
                    if (SendLoginSAnswer(outerSocket, ans_err) < 0)
                        {
                        close(outerSocket);
                        continue;
                        }
                    WriteLogAccessFailed(adminIP);
                    }
                }
            else
                {
                WriteLogAccessFailed(adminIP);
                }
            }
        else
            {
            WriteLogAccessFailed(adminIP);
            if (SendHdrAnswer(outerSocket, ans_err) < 0)
                {
                close(outerSocket);
                continue;
                }
            }
        }
    else
        {
        WriteLogAccessFailed(adminIP);
        }
    printfd(__FILE__, "Successfull connection from %s\n", inet_ntostring(outerAddr.sin_addr.s_addr).c_str());
    close(outerSocket);
    }
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::RecvHdr(int sock)
{
char buf[sizeof(STG_HEADER)];
memset(buf, 0, sizeof(STG_HEADER));
size_t stgHdrLen = sizeof(STG_HEADER) - 1; // Without 0-char
size_t pos = 0;
while (pos < stgHdrLen)
    {
    if (!WaitPackets(sock))
        {
        state = confHdr;
        SendError("Bad request");
        return -1;
        }
    ssize_t ret = recv(sock, &buf[pos], static_cast<int>(stgHdrLen) - static_cast<int>(pos), 0);
    if (ret <= 0)
        {
	if (ret < 0)
	    logger("recv error: %s", strerror(errno));
        state = confHdr;
        return -1;
        }
    pos += ret;
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
if (err)
    {
    if (send(sock, ERR_HEADER, sizeof(ERR_HEADER) - 1, 0) < 0)
        {
        logger("send error: %s", strerror(errno));
        return -1;
        }
    }
else
    {
    if (send(sock, OK_HEADER, sizeof(OK_HEADER) - 1, 0) < 0)
        {
        logger("send error: %s", strerror(errno));
        return -1;
        }
    }

return 0;
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::RecvLogin(int sock)
{
char login[ADM_LOGIN_LEN + 1];

memset(login, 0, ADM_LOGIN_LEN + 1);

size_t pos = 0;
while (pos < ADM_LOGIN_LEN) {
    if (!WaitPackets(sock))
        {
        state = confHdr;
        return ENODATA;
        }

    ssize_t ret = recv(sock, &login[pos], ADM_LOGIN_LEN - static_cast<int>(pos), 0);

    if (ret <= 0)
        {
        // Error in network
	logger("recv error: %s", strerror(errno));
        state = confHdr;
        return ENODATA;
        }

    pos += ret;
}

if (admins->Find(login, &currAdmin))
    {
    // Admin not found
    state = confHdr;
    return ENODATA;
    }

currAdmin->SetIP(adminIP);
adminLogin = login;
state = confLoginCipher;
return 0;
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::SendLoginAnswer(int sock)
{
if (send(sock, OK_LOGIN, sizeof(OK_LOGIN) - 1, 0) < 0)
    {
    logger("Send OK_LOGIN error in SendLoginAnswer.");
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::RecvLoginS(int sock)
{
char loginS[ADM_LOGIN_LEN + 1];
memset(loginS, 0, ADM_LOGIN_LEN + 1);

size_t pos = 0;
while (pos < ADM_LOGIN_LEN)
    {
    if (!WaitPackets(sock))
        {
        state = confHdr;
        return ENODATA;
        }

    ssize_t ret = recv(sock, &loginS[pos], ADM_LOGIN_LEN - static_cast<int>(pos), 0);

    if (ret <= 0)
        {
        // Network error
        printfd(__FILE__, "recv error: '%s'\n", strerror(errno));
	logger("recv error: %s", strerror(errno));
        state = confHdr;
        return ENODATA;
        }

    pos += ret;
    }

if (currAdmin->GetLogin().empty())
    {
    state = confHdr;
    return ENODATA;
    }

BLOWFISH_CTX ctx;
EnDecodeInit(currAdmin->GetPassword().c_str(), ADM_PASSWD_LEN, &ctx);

char login[ADM_LOGIN_LEN + 1];
for (size_t i = 0; i < ADM_LOGIN_LEN / 8; i++)
    {
    DecodeString(login + i * 8, loginS + i * 8, &ctx);
    }

if (currAdmin == admins->GetNoAdmin())
    {
    // If there are no admins registered in the system - give access with any password
    state = confData;
    return 0;
    }

if (strncmp(currAdmin->GetLogin().c_str(), login, ADM_LOGIN_LEN) != 0)
    {
    state = confHdr;
    return ENODATA;
    }

state = confData;
adminPassword = currAdmin->GetPassword();
return 0;
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::SendLoginSAnswer(int sock, int err)
{
if (err)
    {
    if (send(sock, ERR_LOGINS, sizeof(ERR_LOGINS) - 1, 0) < 0)
        {
        logger("send error: %s", strerror(errno));
        return -1;
        }
    }
else
    {
    if (send(sock, OK_LOGINS, sizeof(OK_LOGINS) - 1, 0) < 0)
        {
        logger("send error: %s", strerror(errno));
        return -1;
        }
    }
return 0;
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::RecvData(int sock)
{
requestList.clear();
BLOWFISH_CTX ctx;

EnDecodeInit(currAdmin->GetPassword().c_str(), ADM_PASSWD_LEN, &ctx);

while (1)
    {
    bool done = false;
    char bufferS[8];
    size_t pos = 0;
    while (pos < sizeof(bufferS))
        {
        if (!WaitPackets(sock))
            {
            done = true;
            break;
            }

        ssize_t ret = recv(sock, &bufferS[pos], sizeof(bufferS) - static_cast<int>(pos), 0);
        if (ret < 0)
            {
            // Network error
	    logger("recv error: %s", strerror(errno));
            printfd(__FILE__, "recv error: '%s'\n", strerror(errno));
            return -1;
            }

        if (ret == 0)
            {
            done = true;
            break;
            }

        pos += ret;
        }

    char buffer[8];
    buffer[7] = 0;

    DecodeString(buffer, bufferS, &ctx);
    requestList.push_back(std::string(buffer, pos));

    if (done || memchr(buffer, 0, pos) != NULL)
        {
        // End of data
        if (ParseCommand())
            {
            SendError("Bad command");
            }
        return SendDataAnswer(sock);
        }
    }
//return 0;
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::SendDataAnswer(int sock)
{
std::list<std::string>::iterator li;
li = answerList.begin();

BLOWFISH_CTX ctx;

char buff[8];
char buffS[8];
int n = 0;
int k = 0;

EnDecodeInit(adminPassword.c_str(), ADM_PASSWD_LEN, &ctx);

while (li != answerList.end())
    {
    while ((*li).c_str()[k])
        {
        buff[n % 8] = (*li).c_str()[k];
        n++;
        k++;

        if (n % 8 == 0)
            {
            EncodeString(buffS, buff, &ctx);
            int ret = static_cast<int>(send(sock, buffS, 8, 0));
            if (ret < 0)
                {
                return -1;
                }
            }
        }
    k = 0;// new node
    ++li;
    }

if (answerList.empty()) {
    return 0;
}

buff[n % 8] = 0;
EncodeString(buffS, buff, &ctx);

answerList.clear();

return static_cast<int>(send(sock, buffS, 8, 0));
}
//-----------------------------------------------------------------------------
void CONFIGPROTO::SendError(const char * text)
{
char s[255];
answerList.clear();
snprintf(s, 255, "<Error value=\"%s\"/>", text);
answerList.push_back(s);
}
//-----------------------------------------------------------------------------
void CONFIGPROTO::WriteLogAccessFailed(uint32_t ip)
{
logger("Admin's connection failed. IP %s", inet_ntostring(ip).c_str());
}
//-----------------------------------------------------------------------------
