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

#pragma once

#include <string>
#include <cstdint>

namespace STG
{

class NetTransact
{
    public:
        using Callback = bool (*)(const std::string&, bool, void *);

        NetTransact(const std::string& server, uint16_t port,
                    const std::string& login, const std::string& password);
        NetTransact(const std::string& server, uint16_t port,
                    const std::string& localAddress, uint16_t localPort,
                    const std::string& login, const std::string& password);
        ~NetTransact();

        int Transact(const std::string& request, Callback f, void* data);
        const std::string & GetError() const { return m_errorMsg; }

        int  Connect();
        void Disconnect();

    private:
        int  TxHeader();
        int  RxHeaderAnswer();

        int  TxLogin();
        int  RxLoginAnswer();

        int  TxLoginS();
        int  RxLoginSAnswer();

        int  TxData(const std::string& text);
        int  RxDataAnswer(Callback f, void* data);

        std::string m_server;
        uint16_t  m_port;
        std::string m_localAddress;
        uint16_t m_localPort;
        std::string m_login;
        std::string m_password;
        int m_sock;
        std::string m_errorMsg;

        static bool TxCrypto(const void * block, size_t size, void * data);
        static bool RxCrypto(const void * block, size_t size, void * data);
};

} // namespace STG
