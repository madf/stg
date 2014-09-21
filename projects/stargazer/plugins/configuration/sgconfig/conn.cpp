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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#include "conn.h"

#include "parser.h"

#include "parser_server_info.h"
#include "parser_admins.h"
#include "parser_tariffs.h"
#include "parser_users.h"
#include "parser_message.h"
#include "parser_auth_by.h"
#include "parser_user_info.h"

#include "stg/settings.h"
#include "stg/admins.h"
#include "stg/users.h"
#include "stg/tariffs.h"
#include "stg/admin.h"
#include "stg/blowfish.h"
#include "stg/bfstream.h"
#include "stg/common.h"

#include <cassert>

#include <unistd.h>
#include <sys/socket.h>

using STG::Conn;

const char Conn::STG_HEADER[] = "SG04";
const char Conn::OK_HEADER[] = "OKHD";
const char Conn::ERR_HEADER[] = "ERHD";
const char Conn::OK_LOGIN[] = "OKLG";
const char Conn::ERR_LOGIN[] = "ERLG";
const char Conn::OK_LOGINS[] = "OKLS";
const char Conn::ERR_LOGINS[] = "ERLS";

Conn::Conn(const SETTINGS & settings,
           ADMINS & admins,
           USERS & users,
           TARIFFS & tariffs,
           int sock, const sockaddr_in& addr)
    : m_settings(settings),
      m_admins(admins),
      m_users(users),
      m_tariffs(tariffs),
      m_admin(NULL),
      m_sock(sock),
      m_addr(addr),
      m_keepAlive(false),
      m_parser(NULL),
      m_xmlParser(XML_ParserCreate(NULL)),
      m_state(HEADER),
      m_buffer(m_header),
      m_bufferSize(sizeof(m_header)),
      m_stream(NULL),
      m_dataState(false, *this)
{
    if (m_xmlParser == NULL)
        throw Error("Failed to create XML parser.");

    XML_ParserReset(m_xmlParser, NULL);
    XML_SetElementHandler(m_xmlParser, ParseXMLStart, ParseXMLEnd);
    XML_SetUserData(m_xmlParser, this);

    /*m_parsers.push_back(new STG::PARSER::GET_SERVER_INFO(m_settings, m_users, m_tariffs));

    m_parsers.push_back(new PARSER_GET_USERS);
    m_parsers.push_back(new PARSER_GET_USER);
    m_parsers.push_back(new PARSER_CHG_USER);
    m_parsers.push_back(new PARSER_ADD_USER);
    m_parsers.push_back(new PARSER_DEL_USER);
    m_parsers.push_back(new PARSER_CHECK_USER);
    m_parsers.push_back(new PARSER_SEND_MESSAGE);
    m_parsers.push_back(new PARSER_AUTH_BY);
    m_parsers.push_back(new PARSER_USER_INFO);

    m_parsers.push_back(new PARSER_GET_TARIFFS);
    m_parsers.push_back(new PARSER_ADD_TARIFF);
    m_parsers.push_back(new PARSER_DEL_TARIFF);
    m_parsers.push_back(new PARSER_CHG_TARIFF);

    m_parsers.push_back(new PARSER_GET_ADMINS);
    m_parsers.push_back(new PARSER_CHG_ADMIN);
    m_parsers.push_back(new PARSER_DEL_ADMIN);
    m_parsers.push_back(new PARSER_ADD_ADMIN);*/
}

Conn::~Conn()
{
    shutdown(m_sock, SHUT_RDWR);
    close(m_sock);

    /*std::map<std::string, BASE_PARSER *>::iterator it(m_parsers.begin());
    for (; it != m_parsers.end(); ++it)
        delete it->second;*/
    XML_ParserFree(m_xmlParser);
}

bool Conn::Read()
{
    ssize_t res = read(m_sock, m_buffer, m_bufferSize);
    if (res < 0)
    {
        // TODO: log it
        return false;
    }
    m_bufferSize -= res;
    return HandleBuffer(res);
}

bool Conn::WriteAnswer(const void* buffer, size_t size)
{
    ssize_t res = write(m_sock, buffer, size);
    if (res < 0)
    {
        // TODO: log it
        return false;
    }
    return true;
}

BASE_PARSER * Conn::GetParser(const std::string & tag)
{
    if (strcasecmp(tag.c_str(), "getserverinfo") == 0)
        return new STG::PARSER::GET_SERVER_INFO(*m_admin, m_settings, m_users, m_tariffs);
    if (strcasecmp(tag.c_str(), "getadmins") == 0)
        return new STG::PARSER::GET_ADMINS(*m_admin, m_admins);
    if (strcasecmp(tag.c_str(), "addadmin") == 0)
        return new STG::PARSER::ADD_ADMIN(*m_admin, m_admins);
    if (strcasecmp(tag.c_str(), "deladmin") == 0)
        return new STG::PARSER::DEL_ADMIN(*m_admin, m_admins);
    if (strcasecmp(tag.c_str(), "chgadmin") == 0)
        return new STG::PARSER::CHG_ADMIN(*m_admin, m_admins);
    return NULL;
}

bool Conn::HandleBuffer(size_t size)
{
    if (m_state == DATA)
        return HandleData(size);

    if (m_bufferSize > 0)
        return true;

    switch (m_state)
    {
        case HEADER: return HandleHeader();
        case LOGIN: return HandleLogin();
        case CRYPTO_LOGIN: return HandleCryptoLogin();
        default: return true;
    }

    return true;
}

bool Conn::HandleHeader()
{
    if (strncmp(m_header, STG_HEADER, sizeof(m_header)) != 0)
    {
        WriteAnswer(ERR_HEADER, sizeof(ERR_HEADER));
        // TODO: log it
        m_state = ERROR;
        return false;
    }
    m_state = LOGIN;
    m_buffer = m_login;
    m_bufferSize = sizeof(m_login);
    return WriteAnswer(OK_HEADER, sizeof(OK_HEADER));
}

bool Conn::HandleLogin()
{
    if (m_admins.Find(m_login, &m_admin)) // ADMINS::Find returns true on error.
    {
        WriteAnswer(ERR_LOGIN, sizeof(ERR_LOGIN));
        // TODO: log it
        m_state = ERROR;
        return false;
    }
    m_admin->SetIP(IP());
    m_state = CRYPTO_LOGIN;
    m_buffer = m_cryptoLogin;
    m_bufferSize = sizeof(m_cryptoLogin);
    return WriteAnswer(OK_LOGIN, sizeof(OK_LOGIN));
}

bool Conn::HandleCryptoLogin()
{
    char login[ADM_LOGIN_LEN + 1];
    BLOWFISH_CTX ctx;
    InitContext(m_admin->GetPassword().c_str(), ADM_PASSWD_LEN, &ctx);
    DecryptString(login, m_cryptoLogin, ADM_LOGIN_LEN, &ctx);

    if (strncmp(m_login, login, sizeof(login)) != 0)
    {
        WriteAnswer(ERR_LOGINS, sizeof(ERR_LOGINS));
        // TODO: log it
        m_state = ERROR;
        return false;
    }

    m_state = DATA;
    m_buffer = m_data;
    m_bufferSize = sizeof(m_data);
    m_stream = new STG::DECRYPT_STREAM(m_admin->GetPassword(), DataCallback, &m_dataState);
    return WriteAnswer(OK_LOGINS, sizeof(OK_LOGINS));
}

bool Conn::HandleData(size_t size)
{
    m_stream->Put(m_buffer, size, size == 0 || memchr(m_buffer, 0, size) != NULL);
    return m_stream->IsOk();
}

bool Conn::DataCallback(const void * block, size_t size, void * data)
{
    assert(data != NULL);
    DataState& state = *static_cast<DataState *>(data);

    if (XML_Parse(state.conn.m_xmlParser,
                  static_cast<const char *>(block),
                  size, state.final) == XML_STATUS_ERROR)
    {
        // TODO: log it
        printfd(__FILE__, "XML parse error at line %d, %d: %s. Is final: %d",
                  static_cast<int>(XML_GetCurrentLineNumber(state.conn.m_xmlParser)),
                  static_cast<int>(XML_GetCurrentColumnNumber(state.conn.m_xmlParser)),
                  XML_ErrorString(XML_GetErrorCode(state.conn.m_xmlParser)), (int)state.final);
        return false;
    }

    if (state.final)
    {
        if (!state.conn.WriteResponse())
        {
            // TODO: log it
            state.conn.m_state = ERROR;
            return false;
        }
        state.conn.m_state = DONE;
    }

    return true;
}

void Conn::ParseXMLStart(void * data, const char * el, const char ** attr)
{
    assert(data != NULL);
    Conn & conn = *static_cast<Conn *>(data);

    if (conn.m_parser == NULL)
        conn.m_parser = conn.GetParser(el);

    if (conn.m_parser == NULL)
    {
        // TODO: log it
        conn.m_state = ERROR;
        return;
    }

    conn.m_parser->Start(data, el, attr);
}

void Conn::ParseXMLEnd(void * data, const char * el)
{
    assert(data != NULL);
    Conn & conn = *static_cast<Conn *>(data);

    if (conn.m_parser == NULL)
    {
        // TODO: log it
        conn.m_state = ERROR;
        return;
    }

    conn.m_parser->End(data, el);
}

bool Conn::WriteResponse()
{
    STG::ENCRYPT_STREAM stream(m_admin->GetPassword(), WriteCallback, this);
    const std::string & answer = m_parser->GetAnswer();
    stream.Put(answer.c_str(), answer.length() + 1 /* including \0 */, true /* final */);
    return stream.IsOk();
}

bool Conn::WriteCallback(const void * block, size_t size, void * data)
{
    assert(data != NULL);
    Conn & conn = *static_cast<Conn *>(data);
    return WriteAll(conn.m_sock, block, size);;
}
