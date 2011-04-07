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

/*
 *  Header file for client part of data access via Stargazer for RADIUS
 *
 *  $Revision: 1.4 $
 *  $Date: 2010/04/16 12:30:02 $
 *
 */

#ifndef STG_CLIENT_H
#define STG_CLIENT_H

#include <string>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h> // socklen_t

#include "stg/blowfish.h"
#include "stg/rad_packets.h"

class STG_CLIENT
{
public:
    STG_CLIENT(const std::string & host, uint16_t port, uint16_t lp, const std::string & pass);
    ~STG_CLIENT();

    std::string GetUserPassword() const;

    int Authorize(const std::string & login, const std::string & svc);
    int Authenticate(const std::string & login, const std::string & svc);
    int PostAuthenticate(const std::string & login, const std::string & svc);
    int Account(const std::string & type, const std::string & login, const std::string & svc, const std::string & sessid);

    uint32_t GetFramedIP() const;

    const std::string & GetError() const { return errorStr; };

private:
    uint16_t localPort;
    std::string password;
    int sock;
    std::string errorStr;

    struct sockaddr_in outerAddr;

    std::string userPassword;

    uint32_t framedIP;

    BLOWFISH_CTX ctx;

    int PrepareNet();

    int Request(RAD_PACKET * packet, const std::string & login, const std::string & svc, uint8_t packetType);

    int RecvData(RAD_PACKET * packet);
    int Send(const RAD_PACKET & packet);
};

#endif
