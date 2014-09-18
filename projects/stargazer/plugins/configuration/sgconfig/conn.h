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

#ifndef __STG_SGCONFIG_CONN_H__
#define __STG_SGCONFIG_CONN_H__

#include "stg/os_int.h"
#include "stg/const.h"

#include <stdexcept>
#include <string>
#include <map>

#include <expat.h>

#include <netinet/in.h>

class SETTINGS;
class ADMINS;
class USERS;
class TARIFFS;
class ADMIN;
class BASE_PARSER;

namespace STG
{

class DECRYPT_STREAM;

class Conn
{
    public:
        struct Error : public std::runtime_error
        {
            Error(const std::string& message) : runtime_error(message.c_str()) {}
        };

        Conn(const SETTINGS & settings,
             ADMINS & admins,
             USERS & users,
             TARIFFS & tariffs,
             int sock, const sockaddr_in& addr);
        ~Conn();

        int Sock() const { return m_sock; }
        uint32_t IP() const { return *(uint32_t *)(&m_addr.sin_addr); }
        uint16_t Port() const { return m_addr.sin_port; }

        bool Read();

        bool IsOk() const { return m_state != ERROR; }
        bool IsKeepAlive() const { return m_keepAlive; }

    private:

        static const char STG_HEADER[5];
        static const char OK_HEADER[5];
        static const char ERR_HEADER[5];
        static const char OK_LOGIN[5];
        static const char ERR_LOGIN[5];
        static const char OK_LOGINS[5];
        static const char ERR_LOGINS[5];

        const SETTINGS & m_settings;
        ADMINS & m_admins;
        USERS & m_users;
        TARIFFS & m_tariffs;

        ADMIN * m_admin;

        int m_sock;
        sockaddr_in m_addr;
        bool m_keepAlive;

        BASE_PARSER * m_parser;

        XML_Parser m_xmlParser;

        enum { HEADER, LOGIN, CRYPTO_LOGIN, DATA, ERROR } m_state;

        void * m_buffer;
        size_t m_bufferSize;
        char m_header[sizeof(STG_HEADER)];
        char m_login[ADM_LOGIN_LEN + 1];
        char m_cryptoLogin[ADM_LOGIN_LEN + 1];
        char m_data[1024];
        STG::DECRYPT_STREAM * m_stream;

        BASE_PARSER * GetParser(const std::string & tag);

        bool HandleBuffer(size_t size);

        bool HandleHeader();
        bool HandleLogin();
        bool HandleCryptoLogin();
        bool HandleData(size_t size);

        struct DataState
        {
            DataState(bool f, Conn & c) : final(f), conn(c) {}
            bool final;
            Conn & conn;
        } m_dataState;

        static bool DataCallback(const void * block, size_t size, void * data);
        static void ParseXMLStart(void * data, const char * el, const char ** attr);
        static void ParseXMLEnd(void * data, const char * el);
};

}

#endif
