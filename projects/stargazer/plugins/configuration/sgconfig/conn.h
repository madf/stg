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

#pragma once

#include "parser.h"

#include "dumphelpers.h"

#include "stg/const.h"

#include <stdexcept>
#include <string>
#include <cstdint>

#include <expat.h>

#include <netinet/in.h>

namespace STG
{

struct Settings;
struct Admins;
struct Users;
struct Tariffs;
struct Admin;
class PluginLogger;

class DECRYPT_STREAM;

class Conn
{
    public:
        struct Error : public std::runtime_error
        {
            explicit Error(const std::string& message) : runtime_error(message.c_str()) {}
        };

        Conn(const BASE_PARSER::REGISTRY & registry,
             Admins & admins, int sock, const sockaddr_in& addr,
             PluginLogger & logger);
        ~Conn();

        int Sock() const { return m_sock; }
        uint32_t IP() const { return *reinterpret_cast<const uint32_t *>(&m_addr.sin_addr); }
        uint16_t Port() const { return ntohs(m_addr.sin_port); }

        std::string endpoint() const { return inet_ntostring(IP()) + ":" + std::to_string(Port()); }

        bool Read();

        bool IsOk() const { return m_state != ERROR; }
        bool IsDone() const { return m_state == DONE; }
        bool IsKeepAlive() const { return m_keepAlive; }

        void SetKeepAlive() { m_keepAlive = true; }

    private:

        static const char STG_HEADER[5];
        static const char OK_HEADER[5];
        static const char ERR_HEADER[5];
        static const char OK_LOGIN[5];
        static const char ERR_LOGIN[5];
        static const char OK_LOGINS[5];
        static const char ERR_LOGINS[5];

        const BASE_PARSER::REGISTRY & m_registry;

        Admins & m_admins;

        Admin * m_admin;

        int m_sock;
        sockaddr_in m_addr;
        bool m_keepAlive;

        BASE_PARSER * m_parser;

        XML_Parser m_xmlParser;

        enum { HEADER, LOGIN, CRYPTO_LOGIN, DATA, DONE, ERROR } m_state;

        void * m_buffer;
        size_t m_bufferSize;
        char m_header[sizeof(STG_HEADER) - 1]; // Without \0
        char m_login[ADM_LOGIN_LEN]; // Without \0
        char m_cryptoLogin[ADM_LOGIN_LEN]; // Without \0
        char m_data[1024];
        STG::DECRYPT_STREAM * m_stream;
        PluginLogger &  m_logger;

        BASE_PARSER * GetParser(const std::string & tag) const;

        bool HandleBuffer(size_t size);

        bool HandleHeader();
        bool HandleLogin();
        bool HandleCryptoLogin();
        bool HandleData(size_t size);

        bool WriteAnswer(const void* buffer, size_t size);
        bool WriteResponse();

        void Log(const char * file, const std::string & message);

        struct DataState
        {
            DataState(bool f, Conn & c) : final(f), conn(c) {}
            bool final;
            Conn & conn;
        } m_dataState;

#ifdef DUMPCRYPTO
        Dumper m_dumper;
#endif

        static bool DataCallback(const void * block, size_t size, void * data);
        static void ParseXMLStart(void * data, const char * el, const char ** attr);
        static void ParseXMLEnd(void * data, const char * el);
        static bool WriteCallback(const void * block, size_t size, void * data);
};

}
