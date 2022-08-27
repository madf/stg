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
    bool last;
    NetTransact::Callback callback;
    void* callbackData;
    NetTransact* nt;
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
NetTransact::NetTransact(const std::string& s, uint16_t p,
                         const std::string& l, const std::string& pwd)
    : m_server(s),
      m_port(p),
      m_localPort(0),
      m_login(l),
      m_password(pwd),
      m_sock(-1)
{
}
//---------------------------------------------------------------------------
NetTransact::NetTransact(const std::string& s, uint16_t p,
                         const std::string& la, uint16_t lp,
                         const std::string& l, const std::string& pwd)
    : m_server(s),
      m_port(p),
      m_localAddress(la),
      m_localPort(lp),
      m_login(l),
      m_password(pwd),
      m_sock(-1)
{
}
//---------------------------------------------------------------------------
NetTransact::~NetTransact()
{
    Disconnect();
}
//---------------------------------------------------------------------------
int NetTransact::Connect()
{
    m_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (m_sock < 0)
    {
        m_errorMsg = CREATE_SOCKET_ERROR;
        return st_conn_fail;
    }

    if (!m_localAddress.empty())
    {
        if (m_localPort == 0)
            m_localPort = m_port;

        uint32_t ip = inet_addr(m_localAddress.c_str());

        if (ip == INADDR_NONE)
        {
            auto phe = gethostbyname(m_localAddress.c_str());
            if (phe == NULL)
            {
                m_errorMsg = "Can not reslove '" + m_localAddress + "'";
                return st_dns_err;
            }

            struct hostent he;
            memcpy(&he, phe, sizeof(he));
            ip = *reinterpret_cast<uint32_t*>(he.h_addr_list[0]);
        }

        struct sockaddr_in localAddr;
        memset(&localAddr, 0, sizeof(localAddr));
        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons(m_localPort);
        localAddr.sin_addr.s_addr = ip;

        if (bind(m_sock, reinterpret_cast<sockaddr*>(&localAddr), sizeof(localAddr)) < 0)
        {
            m_errorMsg = BIND_FAILED;
            return st_conn_fail;
        }
    }

    struct sockaddr_in outerAddr;
    memset(&outerAddr, 0, sizeof(outerAddr));

    uint32_t ip = inet_addr(m_server.c_str());

    if (ip == INADDR_NONE)
    {
        auto phe = gethostbyname(m_server.c_str());
        if (phe == NULL)
        {
            m_errorMsg = "Can not reslove '" + m_server + "'";
            return st_dns_err;
        }

        struct hostent he;
        memcpy(&he, phe, sizeof(he));
        ip = *reinterpret_cast<uint32_t*>(he.h_addr_list[0]);
    }

    outerAddr.sin_family = AF_INET;
    outerAddr.sin_port = htons(m_port);
    outerAddr.sin_addr.s_addr = ip;

    if (connect(m_sock, reinterpret_cast<sockaddr *>(&outerAddr), sizeof(outerAddr)) < 0)
    {
        m_errorMsg = CONNECT_FAILED;
        return st_conn_fail;
    }

    return st_ok;
}
//---------------------------------------------------------------------------
void NetTransact::Disconnect()
{
    if (m_sock != -1)
    {
        shutdown(m_sock, SHUT_RDWR);
        close(m_sock);
        m_sock = -1;
    }
}
//---------------------------------------------------------------------------
int NetTransact::Transact(const std::string& request, Callback callback, void* data)
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
int NetTransact::TxHeader()
{
    if (!WriteAll(m_sock, STG_HEADER, strlen(STG_HEADER)))
    {
        m_errorMsg = SEND_HEADER_ERROR;
        return st_send_fail;
    }

    return st_ok;
}
//---------------------------------------------------------------------------
int NetTransact::RxHeaderAnswer()
{
    char buffer[sizeof(STG_HEADER) + 1];

    if (!ReadAll(m_sock, buffer, strlen(OK_HEADER)))
    {
        m_errorMsg = RECV_HEADER_ANSWER_ERROR;
        return st_recv_fail;
    }

    if (strncmp(OK_HEADER, buffer, strlen(OK_HEADER)) == 0)
        return st_ok;

    if (strncmp(ERR_HEADER, buffer, strlen(ERR_HEADER)) == 0)
    {
        m_errorMsg = INCORRECT_HEADER;
        return st_header_err;
    }

    m_errorMsg = UNKNOWN_ERROR;
    return st_unknown_err;
}
//---------------------------------------------------------------------------
int NetTransact::TxLogin()
{
    char loginZ[ADM_LOGIN_LEN + 1];
    memset(loginZ, 0, ADM_LOGIN_LEN + 1);
    strncpy(loginZ, m_login.c_str(), ADM_LOGIN_LEN);

    if (!WriteAll(m_sock, loginZ, ADM_LOGIN_LEN))
    {
        m_errorMsg = SEND_LOGIN_ERROR;
        return st_send_fail;
    }

    return st_ok;
}
//---------------------------------------------------------------------------
int NetTransact::RxLoginAnswer()
{
    char buffer[sizeof(OK_LOGIN) + 1];

    if (!ReadAll(m_sock, buffer, strlen(OK_LOGIN)))
    {
        m_errorMsg = RECV_LOGIN_ANSWER_ERROR;
        return st_recv_fail;
    }

    if (strncmp(OK_LOGIN, buffer, strlen(OK_LOGIN)) == 0)
        return st_ok;

    if (strncmp(ERR_LOGIN, buffer, strlen(ERR_LOGIN)) == 0)
    {
        m_errorMsg = INCORRECT_LOGIN;
        return st_login_err;
    }

    m_errorMsg = UNKNOWN_ERROR;
    return st_unknown_err;
}
//---------------------------------------------------------------------------
int NetTransact::TxLoginS()
{
    char loginZ[ADM_LOGIN_LEN + 1];
    memset(loginZ, 0, ADM_LOGIN_LEN + 1);

    BLOWFISH_CTX ctx;
    InitContext(m_password.c_str(), PASSWD_LEN, &ctx);
    EncryptString(loginZ, m_login.c_str(), std::min<size_t>(m_login.length() + 1, ADM_LOGIN_LEN), &ctx);
    if (!WriteAll(m_sock, loginZ, ADM_LOGIN_LEN))
    {
        m_errorMsg = SEND_LOGIN_ERROR;
        return st_send_fail;
    }

    return st_ok;
}
//---------------------------------------------------------------------------
int NetTransact::RxLoginSAnswer()
{
    char buffer[sizeof(OK_LOGINS) + 1];

    if (!ReadAll(m_sock, buffer, strlen(OK_LOGINS)))
    {
        m_errorMsg = RECV_LOGIN_ANSWER_ERROR;
        return st_recv_fail;
    }

    if (strncmp(OK_LOGINS, buffer, strlen(OK_LOGINS)) == 0)
        return st_ok;

    if (strncmp(ERR_LOGINS, buffer, strlen(ERR_LOGINS)) == 0)
    {
        m_errorMsg = INCORRECT_LOGIN;
        return st_logins_err;
    }

    m_errorMsg = UNKNOWN_ERROR;
    return st_unknown_err;
}
//---------------------------------------------------------------------------
int NetTransact::TxData(const std::string& text)
{
    STG::ENCRYPT_STREAM stream(m_password, TxCrypto, this);
    stream.Put(text.c_str(), text.length() + 1, true);
    if (!stream.IsOk())
    {
        m_errorMsg = SEND_DATA_ERROR;
        return st_send_fail;
    }

    return st_ok;
}
//---------------------------------------------------------------------------
int NetTransact::RxDataAnswer(Callback callback, void* data)
{
    ReadState state = {false, callback, data, this};
    STG::DECRYPT_STREAM stream(m_password, RxCrypto, &state);
    while (!state.last)
    {
        char buffer[1024];
        ssize_t res = read(m_sock, buffer, sizeof(buffer));
        if (res < 0)
        {
            m_errorMsg = RECV_DATA_ANSWER_ERROR;
            return st_recv_fail;
        }
        stream.Put(buffer, res, res == 0);
        if (!stream.IsOk())
            return st_xml_parse_error;
    }

    return st_ok;
}
//---------------------------------------------------------------------------
bool NetTransact::TxCrypto(const void* block, size_t size, void* data)
{
    assert(data != NULL);
    auto& nt = *static_cast<NetTransact*>(data);
    if (!WriteAll(nt.m_sock, block, size))
        return false;
    return true;
}
//---------------------------------------------------------------------------
bool NetTransact::RxCrypto(const void* block, size_t size, void* data)
{
    assert(data != NULL);
    auto& state = *static_cast<ReadState *>(data);

    const char* buffer = static_cast<const char *>(block);
    for (size_t pos = 0; pos < size; ++pos)
        if (buffer[pos] == 0)
        {
            state.last = true;
            size = pos; // Adjust string size
        }

    if (state.callback)
        if (!state.callback(std::string(buffer, size), state.last, state.callbackData))
            return false;

    return true;
}
