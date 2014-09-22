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

#include "stg/admins.h"
#include "stg/admin.h"
#include "stg/blowfish.h"
#include "stg/bfstream.h"
#include "stg/common.h"

#include <cassert>
#include <cstring>
#include <cerrno>

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

Conn::Conn(const BASE_PARSER::REGISTRY & registry,
           ADMINS & admins, int sock, const sockaddr_in& addr)
    : m_registry(registry),
      m_admins(admins),
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
}

Conn::~Conn()
{
    shutdown(m_sock, SHUT_RDWR);
    close(m_sock);

    XML_ParserFree(m_xmlParser);
}

bool Conn::Read()
{
    ssize_t res = read(m_sock, m_buffer, m_bufferSize);
    if (res < 0)
    {
        printfd(__FILE__, "Failed to read data from %s:%d: '%s'.\n", inet_ntostring(IP()).c_str(), Port(), strerror(errno));
        m_state = ERROR;
        // TODO: log it
        return false;
    }
    if (res == 0 && m_state != DATA) // EOF is ok for data.
    {
        printfd(__FILE__, "Failed to read data from %s:%d: 'EOF'.\n", inet_ntostring(IP()).c_str(), Port());
        m_state = ERROR;
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

BASE_PARSER * Conn::GetParser(const std::string & tag) const
{
    BASE_PARSER::REGISTRY::const_iterator it = m_registry.find(tag);
    if (it == m_registry.end())
        return NULL;
    return it->second->create(*m_admin);
}

bool Conn::HandleBuffer(size_t size)
{
    printfd(__FILE__, "Got %d bytes. State: %s.\n", size, (m_state == DATA ? "DATA" : (m_state == HEADER ? "HEADER" : (m_state == LOGIN ? "LOGIN" : (m_state == CRYPTO_LOGIN ? "CRYPTO_LOGIN" : (m_state == DONE ? "DONE" : "ERROR"))))));
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
        printfd(__FILE__, "Wrong header from %s:%d.\n", inet_ntostring(IP()).c_str(), Port());
        WriteAnswer(ERR_HEADER, sizeof(ERR_HEADER) - 1); // Without \0
        // TODO: log it
        m_state = ERROR;
        return false;
    }
    m_state = LOGIN;
    m_buffer = m_login;
    m_bufferSize = sizeof(m_login);
    return WriteAnswer(OK_HEADER, sizeof(OK_HEADER) - 1); // Without \0
}

bool Conn::HandleLogin()
{
    if (m_admins.Find(m_login, &m_admin)) // ADMINS::Find returns true on error.
    {
        printfd(__FILE__, "Wrong login ('%s') from %s:%d.\n", m_login, inet_ntostring(IP()).c_str(), Port());
        WriteAnswer(ERR_LOGIN, sizeof(ERR_LOGIN) - 1); // Without \0
        // TODO: log it
        m_state = ERROR;
        return false;
    }
    m_admin->SetIP(IP());
    m_state = CRYPTO_LOGIN;
    m_buffer = m_cryptoLogin;
    m_bufferSize = sizeof(m_cryptoLogin);
    return WriteAnswer(OK_LOGIN, sizeof(OK_LOGIN) - 1); // Without \0
}

bool Conn::HandleCryptoLogin()
{
    char login[ADM_LOGIN_LEN + 1];
    BLOWFISH_CTX ctx;
    InitContext(m_admin->GetPassword().c_str(), ADM_PASSWD_LEN, &ctx);
    DecryptString(login, m_cryptoLogin, ADM_LOGIN_LEN, &ctx);

    if (strncmp(m_login, login, sizeof(login)) != 0)
    {
        printfd(__FILE__, "Wrong password from %s:%d: '%s' != '%s'.\n", inet_ntostring(IP()).c_str(), Port(), login, m_login);
        WriteAnswer(ERR_LOGINS, sizeof(ERR_LOGINS) - 1); // Without \0
        // TODO: log it
        m_state = ERROR;
        return false;
    }

    m_state = DATA;
    m_buffer = m_data;
    m_bufferSize = sizeof(m_data);
    m_stream = new STG::DECRYPT_STREAM(m_admin->GetPassword(), DataCallback, &m_dataState);
    return WriteAnswer(OK_LOGINS, sizeof(OK_LOGINS) - 1); // Without \0
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

    const char * xml = static_cast<const char *>(block);
    size_t length = strnlen(xml, size);

    state.final = state.final || length < size || size == 0;

    if (XML_Parse(state.conn.m_xmlParser, xml, length, state.final) == XML_STATUS_ERROR)
    {
        // TODO: log it
        printfd(__FILE__, "XML parse error at line %d, %d: %s. Is final: %d\n",
                  static_cast<int>(XML_GetCurrentLineNumber(state.conn.m_xmlParser)),
                  static_cast<int>(XML_GetCurrentColumnNumber(state.conn.m_xmlParser)),
                  XML_ErrorString(XML_GetErrorCode(state.conn.m_xmlParser)), (int)state.final);
        printfd(__FILE__, "Data block: '%s' of size %d\n", xml, length);
        state.conn.m_state = ERROR;
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
        printfd(__FILE__, "Failed to find a suitable parser for '%s'.\n", el);
        // TODO: log it
        conn.m_state = ERROR;
        return;
    }
    else
        printfd(__FILE__, "Using parser '%s'.\n", conn.m_parser->GetTag().c_str());

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
    std::string answer;
    if (m_parser != NULL)
        answer = m_parser->GetAnswer();
    else
        answer = "<Error result=\"Unknown command.\"/>";
    printfd(__FILE__, "Writing %d bytes of answer: '%s'\n", answer.length(), answer.c_str());
    stream.Put(answer.c_str(), answer.length() + 1 /* including \0 */, true /* final */);
    return stream.IsOk();
}

bool Conn::WriteCallback(const void * block, size_t size, void * data)
{
    assert(data != NULL);
    Conn & conn = *static_cast<Conn *>(data);
    return WriteAll(conn.m_sock, block, size);;
}
