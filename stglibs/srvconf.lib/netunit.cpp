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

#include "netunit.h"

#include "stg/servconf_types.h"
#include "stg/common.h"
#include "stg/blowfish.h"

#include <algorithm> // std::min

#include <cstdio>
#include <cerrno>
#include <cstring>

#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace STG;

namespace
{

const std::string::size_type MAX_XML_CHUNK_LENGTH = 2048;

}

//---------------------------------------------------------------------------

#define SEND_DATA_ERROR             "Send data error!"
#define RECV_DATA_ANSWER_ERROR      "Recv data answer error!"
#define UNKNOWN_ERROR               "Unknown error!"
#define CONNECT_FAILED              "Connect failed!"
#define BIND_FAILED                 "Bind failed!"
#define INCORRECT_LOGIN             "Incorrect login!"
#define INCORRECT_HEADER            "Incorrect header!"
#define SEND_LOGIN_ERROR            "Send login error!"
#define RECV_LOGIN_ANSWER_ERROR     "Recv login answer error!"
#define CREATE_SOCKET_ERROR         "Create socket failed!"
#define WSASTARTUP_FAILED           "WSAStartup failed!"
#define SEND_HEADER_ERROR           "Send header error!"
#define RECV_HEADER_ANSWER_ERROR    "Recv header answer error!"

//---------------------------------------------------------------------------
NETTRANSACT::NETTRANSACT(const std::string & s, uint16_t p,
                         const std::string & l, const std::string & pwd)
    : server(s),
      port(p),
      localPort(0),
      login(l),
      password(pwd),
      outerSocket(-1)
{
}
//---------------------------------------------------------------------------
NETTRANSACT::NETTRANSACT(const std::string & s, uint16_t p,
                         const std::string & la, uint16_t lp,
                         const std::string & l, const std::string & pwd)
    : server(s),
      port(p),
      localAddress(la),
      localPort(lp),
      login(l),
      password(pwd),
      outerSocket(-1)
{
}
//---------------------------------------------------------------------------
int NETTRANSACT::Connect()
{
outerSocket = socket(PF_INET, SOCK_STREAM, 0);
if (outerSocket < 0)
    {
    errorMsg = CREATE_SOCKET_ERROR;
    return st_conn_fail;
    }

if (!localAddress.empty())
    {
    if (localPort == 0)
        localPort = port;

    unsigned long ip = inet_addr(localAddress.c_str());

    if (ip == INADDR_NONE)
        {
        struct hostent * phe = gethostbyname(localAddress.c_str());
        if (phe == NULL)
            {
            errorMsg = "DNS error.\nCan not reslove " + localAddress;
            return st_dns_err;
            }

        struct hostent he;
        memcpy(&he, phe, sizeof(he));
        ip = *((long *)he.h_addr_list[0]);
        }

    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(localPort);
    localAddr.sin_addr.s_addr = ip;

    if (bind(outerSocket, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0)
        {
        errorMsg = BIND_FAILED;
        close(outerSocket);
        return st_conn_fail;
        }
    }

struct sockaddr_in outerAddr;
memset(&outerAddr, 0, sizeof(outerAddr));

unsigned long ip = inet_addr(server.c_str());

if (ip == INADDR_NONE)
    {
    struct hostent * phe = gethostbyname(server.c_str());
    if (phe == NULL)
        {
        errorMsg = "DNS error.\nCan not reslove " + server;
        return st_dns_err;
        }

    struct hostent he;
    memcpy(&he, phe, sizeof(he));
    ip = *((long *)he.h_addr_list[0]);
    }

outerAddr.sin_family = AF_INET;
outerAddr.sin_port = htons(port);
outerAddr.sin_addr.s_addr = ip;

if (connect(outerSocket, (struct sockaddr *)&outerAddr, sizeof(outerAddr)) < 0)
    {
    errorMsg = CONNECT_FAILED;
    close(outerSocket);
    return st_conn_fail;
    }

return st_ok;
}
//---------------------------------------------------------------------------
void NETTRANSACT::Disconnect()
{
close(outerSocket);
}
//---------------------------------------------------------------------------
int NETTRANSACT::Transact(const std::string & request, CALLBACK callback, void * data)
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

if ((ret = TxData(request)) != st_ok)
    {
    Disconnect();
    return ret;
    }

if ((ret = RxDataAnswer(callback, data)) != st_ok)
    {
    Disconnect();
    return ret;
    }

return st_ok;
}
//---------------------------------------------------------------------------
int NETTRANSACT::TxHeader()
{
if (send(outerSocket, STG_HEADER, strlen(STG_HEADER), 0) <= 0)
    {
    errorMsg = SEND_HEADER_ERROR;
    return st_send_fail;
    }

return st_ok;
}
//---------------------------------------------------------------------------
int NETTRANSACT::RxHeaderAnswer()
{
char buffer[sizeof(STG_HEADER) + 1];

if (recv(outerSocket, buffer, strlen(OK_HEADER), 0) <= 0)
    {
    printf("Receive header answer error: '%s'\n", strerror(errno));
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
memset(loginZ, 0, ADM_LOGIN_LEN);
strncpy(loginZ, login.c_str(), ADM_LOGIN_LEN);

if (send(outerSocket, loginZ, ADM_LOGIN_LEN, 0) <= 0)
    {
    errorMsg = SEND_LOGIN_ERROR;
    return st_send_fail;
    }

return st_ok;
}
//---------------------------------------------------------------------------
int NETTRANSACT::RxLoginAnswer()
{
char buffer[sizeof(OK_LOGIN) + 1];

if (recv(outerSocket, buffer, strlen(OK_LOGIN), 0) <= 0)
    {
    printf("Receive login answer error: '%s'\n", strerror(errno));
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
EncryptString(loginZ, login.c_str(), std::min(login.length() + 1, ADM_LOGIN_LEN), &ctx);
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
char buffer[sizeof(OK_LOGINS) + 1];

if (recv(outerSocket, buffer, strlen(OK_LOGINS), 0) <= 0)
    {
    printf("Receive secret login answer error: '%s'\n", strerror(errno));
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
int NETTRANSACT::TxData(const std::string & text)
{
BLOWFISH_CTX ctx;
InitContext(password.c_str(), PASSWD_LEN, &ctx);
char buffer[text.length() + 9];
EncryptString(buffer, text.c_str(), text.length() + 1, &ctx);
if (send(outerSocket, buffer, sizeof(buffer), 0) <= 0)
    {
    errorMsg = SEND_DATA_ERROR;
    return st_send_fail;
    }
return st_ok;
}
//---------------------------------------------------------------------------
int NETTRANSACT::RxDataAnswer(CALLBACK callback, void * data)
{
BLOWFISH_CTX ctx;
InitContext(password.c_str(), PASSWD_LEN, &ctx);

std::string chunk;
while (true)
    {
    char bufferS[ENC_MSG_LEN];
    size_t toRead = ENC_MSG_LEN;
    while (toRead > 0)
        {
        int ret = recv(outerSocket, &bufferS[ENC_MSG_LEN - toRead], toRead, 0);
        if (ret <= 0)
            {
            printf("Receive data error: '%s'\n", strerror(errno));
            close(outerSocket);
            errorMsg = RECV_DATA_ANSWER_ERROR;
            return st_recv_fail;
            }
        toRead -= ret;
        }

    char buffer[ENC_MSG_LEN];
    DecryptBlock(buffer, bufferS, &ctx);

    bool final = false;
    size_t pos = 0;
    for (; pos < ENC_MSG_LEN && buffer[pos] != 0; pos++) ;
    if (pos < ENC_MSG_LEN && buffer[pos] == 0)
        final = true;

    if (pos > 0)
        chunk.append(&buffer[0], &buffer[pos]);

    if (chunk.length() > MAX_XML_CHUNK_LENGTH || final)
        {
        if (callback)
            if (!callback(chunk, final, data))
                return st_xml_parse_error;
        chunk.clear();
        }

    if (final)
        return st_ok;
    }
}
