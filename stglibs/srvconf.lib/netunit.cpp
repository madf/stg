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
 $Revision: 1.6 $
 $Date: 2009/02/06 10:25:54 $
 $Author: faust $
 */

//---------------------------------------------------------------------------
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

#include "stg/netunit.h"
#include "stg/common.h"

//---------------------------------------------------------------------------

#define SEND_DATA_ERROR             "Send data error!"
#define RECV_DATA_ANSWER_ERROR      "Recv data answer error!"
#define UNKNOWN_ERROR               "Unknown error!"
#define CONNECT_FAILED              "Connect failed!"
#define INCORRECT_LOGIN             "Incorrect login!"
#define INCORRECT_HEADER            "Incorrect header!"
#define SEND_LOGIN_ERROR            "Send login error!"
#define RECV_LOGIN_ANSWER_ERROR     "Recv login answer error!"
#define CREATE_SOCKET_ERROR         "Create socket failed!"
#define WSASTARTUP_FAILED           "WSAStartup failed!"
#define SEND_HEADER_ERROR           "Send header error!"
#define RECV_HEADER_ANSWER_ERROR    "Recv header answer error!"

//---------------------------------------------------------------------------
NETTRANSACT::NETTRANSACT()
    : port(0),
      outerSocket(-1),
      RxCallBack(NULL),
      dataRxCallBack(NULL)
{
}
//---------------------------------------------------------------------------
int NETTRANSACT::Connect()
{
int ret;

outerSocket = socket(PF_INET, SOCK_STREAM, 0);
if (outerSocket < 0)
    {
    errorMsg = CREATE_SOCKET_ERROR;
    return st_conn_fail;
    }

struct sockaddr_in outerAddr;
memset(&outerAddr, 0, sizeof(outerAddr));

struct hostent he;
struct hostent * phe;

unsigned long ip;
ip = inet_addr(server.c_str());

if (ip == INADDR_NONE)
    {
    phe = gethostbyname(server.c_str());
    if (phe == NULL)
        {
        errorMsg = "DNS error.\nCan not reslove " + server;
        return st_dns_err;
        }

    memcpy(&he, phe, sizeof(he));
    ip = *((long*)he.h_addr_list[0]);
    }
outerAddr.sin_family = AF_INET;
outerAddr.sin_port = htons(port);
outerAddr.sin_addr.s_addr = ip;

ret = connect(outerSocket, (struct sockaddr*)&outerAddr, sizeof(outerAddr));

if (ret < 0)
    {
    errorMsg = CONNECT_FAILED;
    close(outerSocket);
    return st_conn_fail;
    }
return st_ok;
}
//---------------------------------------------------------------------------
int NETTRANSACT::Disconnect()
{
close(outerSocket);
return 0;
}
//---------------------------------------------------------------------------
int NETTRANSACT::Transact(const char * data)
{
int ret;
if ((ret = TxHeader()) != st_ok)
    {
    Disconnect();
    return ret;
    }

if ((ret = RxHeaderAnswer()) != st_ok)
    {
    Disconnect();
    return ret;
    }

if ((ret = TxLogin()) != st_ok)
    {
    Disconnect();
    return ret;
    }

if ((ret = RxLoginAnswer()) != st_ok)
    {
    Disconnect();
    return ret;
    }

if ((ret = TxLoginS()) != st_ok)
    {
    Disconnect();
    return ret;
    }

if ((ret = RxLoginSAnswer()) != st_ok)
    {
    Disconnect();
    return ret;
    }

if ((ret = TxData(data)) != st_ok)
    {
    Disconnect();
    return ret;
    }

if ((ret = RxDataAnswer()) != st_ok)
    {
    Disconnect();
    return ret;
    }

return st_ok;
}
//---------------------------------------------------------------------------
int NETTRANSACT::TxHeader()
{
int ret;
ret = send(outerSocket, STG_HEADER, strlen(STG_HEADER), 0);
if (ret <= 0)
    {
    errorMsg = SEND_HEADER_ERROR;
    return st_send_fail;
    }

return st_ok;
}
//---------------------------------------------------------------------------
int NETTRANSACT::RxHeaderAnswer()
{
char buffer[sizeof(STG_HEADER)+1];
int ret;

ret = recv(outerSocket, buffer, strlen(OK_HEADER), 0);
if (ret <= 0)
    {
    errorMsg = RECV_HEADER_ANSWER_ERROR;
    return st_recv_fail;
    }

if (strncmp(OK_HEADER, buffer, strlen(OK_HEADER)) == 0)
    {
    return st_ok;
    }
else
    {
    if (strncmp(ERR_HEADER, buffer, strlen(ERR_HEADER)) == 0)
        {
        errorMsg = INCORRECT_HEADER;
        return st_header_err;
        }
    else
        {
        errorMsg = UNKNOWN_ERROR;
        return st_unknown_err;
        }
    }
}
//---------------------------------------------------------------------------
int NETTRANSACT::TxLogin()
{
char loginZ[ADM_LOGIN_LEN];
int ret;

memset(loginZ, 0, ADM_LOGIN_LEN);
strncpy(loginZ, login.c_str(), ADM_LOGIN_LEN);
ret = send(outerSocket, loginZ, ADM_LOGIN_LEN, 0);

if (ret <= 0)
    {
    errorMsg = SEND_LOGIN_ERROR;
    return st_send_fail;
    }

return st_ok;
}
//---------------------------------------------------------------------------
int NETTRANSACT::RxLoginAnswer()
{
char buffer[sizeof(OK_LOGIN)+1];
int ret;

ret = recv(outerSocket, buffer, strlen(OK_LOGIN), 0);
if (ret <= 0)
    {
    errorMsg = RECV_LOGIN_ANSWER_ERROR;
    return st_recv_fail;
    }

if (strncmp(OK_LOGIN, buffer, strlen(OK_LOGIN)) == 0)
    {
    return st_ok;
    }
else
    {
    if (strncmp(ERR_LOGIN, buffer, strlen(ERR_LOGIN)) == 0)
        {
        errorMsg = INCORRECT_LOGIN;
        return st_login_err;
        }
    else
        {
        errorMsg = UNKNOWN_ERROR;
        return st_unknown_err;
        }
    }
}
//---------------------------------------------------------------------------
int NETTRANSACT::TxLoginS()
{
char loginZ[ADM_LOGIN_LEN];

memset(loginZ, 0, ADM_LOGIN_LEN);
BLOWFISH_CTX ctx;
InitContext(password.c_str(), PASSWD_LEN, &ctx);
EncryptString(loginZ, login.c_str(), std::min<int>(login.length() + 1, ADM_LOGIN_LEN), &ctx);
if (send(outerSocket, loginZ, ADM_LOGIN_LEN, 0) <= 0)
    {
    errorMsg = SEND_LOGIN_ERROR;
    return st_send_fail;
    }
return st_ok;
}
//---------------------------------------------------------------------------
int NETTRANSACT::RxLoginSAnswer()
{
char buffer[sizeof(OK_LOGINS)+1];
int ret;

ret = recv(outerSocket, buffer, strlen(OK_LOGINS), 0);
if (ret <= 0)
    {
    errorMsg = RECV_LOGIN_ANSWER_ERROR;
    return st_recv_fail;
    }

if (strncmp(OK_LOGINS, buffer, strlen(OK_LOGINS)) == 0)
    {
    return st_ok;
    }
else
    {
    if (strncmp(ERR_LOGINS, buffer, strlen(ERR_LOGINS)) == 0)
        {
        errorMsg = INCORRECT_LOGIN;
        return st_logins_err;
        }
    else
        {
        errorMsg = UNKNOWN_ERROR;
        return st_unknown_err;
        }
    }
}
//---------------------------------------------------------------------------
int NETTRANSACT::TxData(const char * text)
{
BLOWFISH_CTX ctx;
InitContext(password.c_str(), PASSWD_LEN, &ctx);
size_t length = strlen(text);
char buffer[length + 9];
memset(buffer, 0, sizeof(buffer));
EncryptString(buffer, text, length + 1, &ctx);
if (send(outerSocket, buffer, sizeof(buffer), 0) <= 0)
    {
    errorMsg = SEND_DATA_ERROR;
    return st_send_fail;
    }
return st_ok;
}
//---------------------------------------------------------------------------
int NETTRANSACT::RxDataAnswer()
{
int n = 0;
int ret;
char bufferS[ENC_MSG_LEN];
char buffer[ENC_MSG_LEN + 1];

BLOWFISH_CTX ctx;
InitContext(password.c_str(), PASSWD_LEN, &ctx);

while (1)
    {
    ret = recv(outerSocket, &bufferS[n++], 1, 0);
    if (ret <= 0)
        {
        close(outerSocket);
        errorMsg = RECV_DATA_ANSWER_ERROR;
        return st_recv_fail;
        }

    if (n == ENC_MSG_LEN)
        {
        n = 0;
        DecryptBlock(buffer, bufferS, &ctx);
        buffer[ENC_MSG_LEN] = 0;

        answerList.push_back(buffer);

        for (int j = 0; j < ENC_MSG_LEN; j++)
            {
            if (buffer[j] == 0)
                {
                if (RxCallBack)
                    if (st_ok != RxCallBack(dataRxCallBack, &answerList))
                        {
                        return st_xml_parse_error;
                        }
                return st_ok;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
void NETTRANSACT::SetLogin(const char * l)
{
login = l;
}
//---------------------------------------------------------------------------
void NETTRANSACT::SetPassword(const char * p)
{
password = p;
}
//---------------------------------------------------------------------------
void NETTRANSACT::SetServer(const char * serverName)
{
server = serverName;
}
//---------------------------------------------------------------------------
void NETTRANSACT::SetServerPort(short unsigned p)
{
port = p;
}
//---------------------------------------------------------------------------
void NETTRANSACT::SetRxCallback(void * data, RxCallback_t cb)
{
RxCallBack = cb;
dataRxCallBack = data;
}
//---------------------------------------------------------------------------
const std::string & NETTRANSACT::GetError() const
{
return errorMsg;
}
//---------------------------------------------------------------------------
void NETTRANSACT::Reset()
{
answerList.clear();
}
//---------------------------------------------------------------------------
