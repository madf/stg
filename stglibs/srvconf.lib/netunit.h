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

#ifndef NetUnitH
#define NetUnitH

#include "stg/os_int.h"

#include <string>

namespace STG
{

class NETTRANSACT
{
public:
    typedef bool (* CALLBACK)(const std::string &, bool, void *);

    NETTRANSACT(const std::string & server, uint16_t port,
                const std::string & login, const std::string & password);
    NETTRANSACT(const std::string & server, uint16_t port,
                const std::string & localAddress, uint16_t localPort,
                const std::string & login, const std::string & password);
    ~NETTRANSACT();
    int Transact(const std::string & request, CALLBACK f, void * data);
    const std::string & GetError() const { return errorMsg; }

    int  Connect();
    void Disconnect();
private:
    int  TxHeader();
    int  RxHeaderAnswer();

    int  TxLogin();
    int  RxLoginAnswer();

    int  TxLoginS();
    int  RxLoginSAnswer();

    int  TxData(const std::string & text);
    int  RxDataAnswer(CALLBACK f, void * data);

    std::string server;
    uint16_t  port;
    std::string localAddress;
    uint16_t localPort;
    std::string login;
    std::string password;
    int sock;
    std::string errorMsg;

    static bool TxCrypto(const void * block, size_t size, void * data);
    static bool RxCrypto(const void * block, size_t size, void * data);
};

} // namespace STG

#endif
