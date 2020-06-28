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
 *    Author : Maksym Mamontov <stg@madf.info>
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

#include "stgpair.h"

class STG_CLIENT
{
public:
    STG_CLIENT(const std::string & host, uint16_t port, const std::string & password);
    ~STG_CLIENT();

    const STG_PAIR * Authorize(const std::string & login, const std::string & service);
    const STG_PAIR * Authenticate(const std::string & login, const std::string & service);
    const STG_PAIR * PostAuth(const std::string & login, const std::string & service);
    const STG_PAIR * PreAcct(const std::string & login, const std::string & service);
    const STG_PAIR * Account(const std::string & type, const std::string & login, const std::string & service, const std::string & sessionId);

private:
    std::string password;

    int PrepareNet();

    int Request(RAD_PACKET * packet, const std::string & login, const std::string & svc, uint8_t packetType);

    int RecvData(RAD_PACKET * packet);
    int Send(const RAD_PACKET & packet);
};

struct STG_CLIENT_ST
{
    public:
        static void Configure(const std::string & host, uint16_t port, const std::string & password);
        static STG_CLIENT * Get();

    private:
        static std::string m_host;
        static uint16_t m_port;
        static std::string m_password;
};

#endif
