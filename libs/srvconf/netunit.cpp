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
#include "stg/bfstream.h"

#include <algorithm> // std::min

#include <cerrno>
#include <cstring>
#include <cassert>

#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

const char STG_HEADER[] = "SG04";
const char OK_HEADER[]  = "OKHD";
const char ERR_HEADER[] = "ERHD";
const char OK_LOGIN[]   = "OKLG";
const char ERR_LOGIN[]  = "ERLG";
const char OK_LOGINS[]  = "OKLS";
const char ERR_LOGINS[] = "ERLS";

using namespace STG;

namespace
{

struct ReadState
{
    bool final;
    NETTRANSACT::CALLBACK callback;
    void * callbackData;
    NETTRANSACT * nt;
};

}

//---------------------------------------------------------------------------

const char SEND_DATA_ERROR[]          = "Error sending data.";
const char RECV_DATA_ANSWER_ERROR[]   = "Error receiving data answer.";
const char UNKNOWN_ERROR[]            = "Unknown error";
const char CONNECT_FAILED[]           = "Failed to connect.";
const char BIND_FAILED[]              = "Failed to bind.";
const char INCORRECT_LOGIN[]          = "Incorrect login.";
const char INCORRECT_HEADER[]         = "Incorrect header.";
const char SEND_LOGIN_ERROR[]         = "Error sending login.";
const char RECV_LOGIN_ANSWER_ERROR[]  = "Error receiving login answer.";
const char CREATE_SOCKET_ERROR[]      = "Failed to create socket.";
const char SEND_HEADER_ERROR[]        = "Error sending header.";
const char RECV_HEADER_ANSWER_ERROR[] = "Error receiving header answer.";

//---------------------------------------------------------------------------
NETTRANSACT::NETTRANSACT(const std::string & s, uint16_t p,
                         const std::string & l, const std::string & pwd)
    : server(s),
      port(p),
      localPort(0),
      login(l),
      password(pwd),
      sock(-1)
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
      sock(-1)
{
}
//---------------------------------------------------------------------------
NETTRANSACT::~NETTRANSACT()
{
Disconnect();
}
//---------------------------------------------------------------------------
int NETTRANSACT::Connect()
{
sock = socket(PF_INET, SOCK_STREAM, 0);
if (sock < 0)
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
            errorMsg = "Can not reslove '" + localAddress + "'";
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

    if (bind(sock, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0)
        {
        errorMsg = BIND_FAILED;
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
        errorMsg = "Can not reslove '" + server + "'";
        return st_dns_err;
        }

    struct hostent he;
    memcpy(&he, phe, sizeof(he));
    ip = *((long *)he.h_addr_list[0]);
    }

outerAddr.sin_family = AF_INET;
outerAddr.sin_port = htons(port);
outerAddr.sin_addr.s_addr = ip;

if (connect(sock, (struct sockaddr *)&outerAddr, sizeof(outerAddr)) < 0)
    {
    errorMsg = CONNECT_FAILED;
    return st_conn_fail;
    }

return st_ok;
}
//---------------------------------------------------------------------------
void NETTRANSACT::Disconnect()
{
if (sock != -1)
    {
    shutdown(sock, SHUT_RDWR);
    close(sock);
    sock = -1;
    }
}
//---------------------------------------------------------------------------
int NETTRANSACT::Transact(const std::string & request, CALLBACK callback, void * data)
{
int ret;
if ((ret = TxHeader()) != st_ok)
    return ret;

if ((ret = RxHeaderAnswer()) != st_ok)
    return ret;

if ((ret = TxLogin()) != st_ok)
    return ret;

if ((ret = RxLoginAnswer()) != st_ok)
    return ret;

if ((ret = TxLoginS()) != st_ok)
    return ret;

if ((ret = RxLoginSAnswer()) != st_ok)
    return ret;

if ((ret = TxData(request)) != st_ok)
    return ret;

if ((ret = RxDataAnswer(callback, data)) != st_ok)
    return ret;

return st_ok;
}
//---------------------------------------------------------------------------
int NETTRANSACT::TxHeader()
{
if (!WriteAll(sock, STG_HEADER, strlen(STG_HEADER)))
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

if (!ReadAll(sock, buffer, strlen(OK_HEADER)))
    {
    errorMsg = RECV_HEADER_ANSWER_ERROR;
    return st_recv_fail;
    }

if (strncmp(OK_HEADER, buffer, strlen(OK_HEADER)) == 0)
    return st_ok;

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
//---------------------------------------------------------------------------
int NETTRANSACT::TxLogin()
{
char loginZ[ADM_LOGIN_LEN + 1];
memset(loginZ, 0, ADM_LOGIN_LEN + 1);
strncpy(loginZ, login.c_str(), ADM_LOGIN_LEN);

if (!WriteAll(sock, loginZ, ADM_LOGIN_LEN))
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

if (!ReadAll(sock, buffer, strlen(OK_LOGIN)))
    {
    errorMsg = RECV_LOGIN_ANSWER_ERROR;
    return st_recv_fail;
    }

if (strncmp(OK_LOGIN, buffer, strlen(OK_LOGIN)) == 0)
    return st_ok;

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
//---------------------------------------------------------------------------
int NETTRANSACT::TxLoginS()
{
char loginZ[ADM_LOGIN_LEN + 1];
memset(loginZ, 0, ADM_LOGIN_LEN + 1);

BLOWFISH_CTX ctx;
InitContext(password.c_str(), PASSWD_LEN, &ctx);
EncryptString(loginZ, login.c_str(), std::min<size_t>(login.length() + 1, ADM_LOGIN_LEN), &ctx);
if (!WriteAll(sock, loginZ, ADM_LOGIN_LEN))
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

if (!ReadAll(sock, buffer, strlen(OK_LOGINS)))
    {
    errorMsg = RECV_LOGIN_ANSWER_ERROR;
    return st_recv_fail;
    }

if (strncmp(OK_LOGINS, buffer, strlen(OK_LOGINS)) == 0)
    return st_ok;

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
//---------------------------------------------------------------------------
int NETTRANSACT::TxData(const std::string & text)
{
STG::ENCRYPT_STREAM stream(password, TxCrypto, this);
stream.Put(text.c_str(), text.length() + 1, true);
if (!stream.IsOk())
    {
    errorMsg = SEND_DATA_ERROR;
    return st_send_fail;
    }

return st_ok;
}
//---------------------------------------------------------------------------
int NETTRANSACT::RxDataAnswer(CALLBACK callback, void * data)
{
ReadState state = {false, callback, data, this};
STG::DECRYPT_STREAM stream(password, RxCrypto, &state);
while (!state.final)
    {
    char buffer[1024];
    ssize_t res = read(sock, buffer, sizeof(buffer));
    if (res < 0)
        {
        errorMsg = RECV_DATA_ANSWER_ERROR;
        return st_recv_fail;
        }
    stream.Put(buffer, res, res == 0);
    if (!stream.IsOk())
        return st_xml_parse_error;
    }

return st_ok;
}
//---------------------------------------------------------------------------
bool NETTRANSACT::TxCrypto(const void * block, size_t size, void * data)
{
assert(data != NULL);
NETTRANSACT & nt = *static_cast<NETTRANSACT *>(data);
if (!WriteAll(nt.sock, block, size))
    return false;
return true;
}
//---------------------------------------------------------------------------
bool NETTRANSACT::RxCrypto(const void * block, size_t size, void * data)
{
assert(data != NULL);
ReadState & state = *static_cast<ReadState *>(data);

const char * buffer = static_cast<const char *>(block);
for (size_t pos = 0; pos < size; ++pos)
    if (buffer[pos] == 0)
        {
        state.final = true;
        size = pos; // Adjust string size
        }

if (state.callback)
    if (!state.callback(std::string(buffer, size), state.final, state.callbackData))
        return false;

return true;
}
