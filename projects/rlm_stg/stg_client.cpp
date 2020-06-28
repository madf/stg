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
 *  Realization of data access via Stargazer for RADIUS
 *
 *  $Revision: 1.8 $
 *  $Date: 2010/04/16 12:30:02 $
 *
 */

#include <netdb.h>
#include <sys/types.h>
#include <unistd.h> // close

#include <cerrno>
#include <cstring>
#include <vector>
#include <utility>

#include <stdexcept>

#include "stg_client.h"

typedef std::vector<std::pair<std::string, std::string> > PAIRS;

//-----------------------------------------------------------------------------

STG_CLIENT::STG_CLIENT(const std::string & host, uint16_t port, uint16_t lp, const std::string & pass)
    : password(pass),
      framedIP(0)
{
/*sock = socket(AF_INET, SOCK_DGRAM, 0);
if (sock == -1)
    {
    std::string message = strerror(errno);
    message = "Socket create error: '" + message + "'";
    throw std::runtime_error(message);
    }

struct hostent * he = NULL;
he = gethostbyname(host.c_str());
if (he == NULL)
    {
    throw std::runtime_error("gethostbyname error");
    }

outerAddr.sin_family = AF_INET;
outerAddr.sin_port = htons(port);
outerAddr.sin_addr.s_addr = *(uint32_t *)he->h_addr;

InitEncrypt(&ctx, password);

PrepareNet();*/
}

STG_CLIENT::~STG_CLIENT()
{
/*close(sock);*/
}

int STG_CLIENT::PrepareNet()
{
return 0;
}

int STG_CLIENT::Send(const RAD_PACKET & packet)
{
/*char buf[RAD_MAX_PACKET_LEN];

Encrypt(&ctx, buf, (char *)&packet, sizeof(RAD_PACKET) / 8);

int res = sendto(sock, buf, sizeof(RAD_PACKET), 0, (struct sockaddr *)&outerAddr, sizeof(outerAddr));

if (res == -1)
    errorStr = "Error sending data";

return res;*/
}

int STG_CLIENT::RecvData(RAD_PACKET * packet)
{
/*char buf[RAD_MAX_PACKET_LEN];
int res;

struct sockaddr_in addr;
socklen_t len = sizeof(struct sockaddr_in);

res = recvfrom(sock, buf, RAD_MAX_PACKET_LEN, 0, reinterpret_cast<struct sockaddr *>(&addr), &len);
if (res == -1)
    {
    errorStr = "Error receiving data";
    return -1;
    }

Decrypt(&ctx, (char *)packet, buf, res / 8);

return 0;*/
}

int STG_CLIENT::Request(RAD_PACKET * packet, const std::string & login, const std::string & svc, uint8_t packetType)
{
/*int res;

memcpy((void *)&packet->magic, (void *)RAD_ID, RAD_MAGIC_LEN);
packet->protoVer[0] = '0';
packet->protoVer[1] = '1';
packet->packetType = packetType;
packet->ip = 0;
strncpy((char *)packet->login, login.c_str(), RAD_LOGIN_LEN);
strncpy((char *)packet->service, svc.c_str(), RAD_SERVICE_LEN);

res = Send(*packet);
if (res == -1)
    return -1;

res = RecvData(packet);
if (res == -1)
    return -1;

if (strncmp((char *)packet->magic, RAD_ID, RAD_MAGIC_LEN))
    {
    errorStr = "Magic invalid. Wanted: '";
    errorStr += RAD_ID;
    errorStr += "', got: '";
    errorStr += (char *)packet->magic;
    errorStr += "'";
    return -1;
    }

return 0;*/
}

//-----------------------------------------------------------------------------

const STG_PAIRS * STG_CLIENT::Authorize(const std::string & login, const std::string & svc)
{
/*RAD_PACKET packet;

userPassword = "";

if (Request(&packet, login, svc, RAD_AUTZ_PACKET))
    return -1;

if (packet.packetType != RAD_ACCEPT_PACKET)
    return -1;

userPassword = (char *)packet.password;*/

PAIRS pairs;
pairs.push_back(std::make_pair("Cleartext-Password", userPassword));

return ToSTGPairs(pairs);
}

const STG_PAIRS * STG_CLIENT::Authenticate(const std::string & login, const std::string & svc)
{
/*RAD_PACKET packet;

userPassword = "";

if (Request(&packet, login, svc, RAD_AUTH_PACKET))
    return -1;

if (packet.packetType != RAD_ACCEPT_PACKET)
    return -1;*/

PAIRS pairs;

return ToSTGPairs(pairs);
}

const STG_PAIRS * STG_CLIENT::PostAuth(const std::string & login, const std::string & svc)
{
/*RAD_PACKET packet;

userPassword = "";

if (Request(&packet, login, svc, RAD_POST_AUTH_PACKET))
    return -1;

if (packet.packetType != RAD_ACCEPT_PACKET)
    return -1;

if (svc == "Framed-User")
    framedIP = packet.ip;
else
    framedIP = 0;*/

PAIRS pairs;
pairs.push_back(std::make_pair("Framed-IP-Address", inet_ntostring(framedIP)));

return ToSTGPairs(pairs);
}

const STG_PAIRS * STG_CLIENT::PreAcct(const std::string & login, const std::String & service)
{
PAIRS pairs;

return ToSTGPairs(pairs);
}

const STG_PAIRS * STG_CLIENT::Account(const std::string & type, const std::string & login, const std::string & svc, const std::string & sessid)
{
/*RAD_PACKET packet;

userPassword = "";
strncpy((char *)packet.sessid, sessid.c_str(), RAD_SESSID_LEN);

if (type == "Start")
    {
    if (Request(&packet, login, svc, RAD_ACCT_START_PACKET))
        return -1;
    }
else if (type == "Stop")
    {
    if (Request(&packet, login, svc, RAD_ACCT_STOP_PACKET))
        return -1;
    }
else if (type == "Interim-Update")
    {
    if (Request(&packet, login, svc, RAD_ACCT_UPDATE_PACKET))
        return -1;
    }
else
    {
    if (Request(&packet, login, svc, RAD_ACCT_OTHER_PACKET))
        return -1;
    }

if (packet.packetType != RAD_ACCEPT_PACKET)
    return -1;*/

PAIRS pairs;

return ToSTGPairs(pairs);
}

//-----------------------------------------------------------------------------

std::string STG_CLIENT_ST::m_host;
uint16_t STG_CLIENT_ST::m_port(6666);
std::string STG_CLIENT_ST::m_password;

//-----------------------------------------------------------------------------

STG_CLIENT * STG_CLIENT_ST::Get()
{
    static STG_CLIENT * stgClient = NULL;
    if ( stgClient == NULL )
        stgClient = new STG_CLIENT(m_host, m_port, m_password);
    return stgClient;
}

void STG_CLIENT_ST::Configure(const std::string & host, uint16_t port, const std::string & password)
{
    m_host = host;
    m_port = port;
    m_password = password;
}

//-----------------------------------------------------------------------------

const STG_PAIR * ToSTGPairs(const PAIRS & source)
{
    STG_PAIR * pairs = new STG_PAIR[source.size() + 1];
    for (size_t pos = 0; pos < source.size(); ++pos) {
        bzero(pairs[pos].key, sizeof(STG_PAIR::key));
        bzero(pairs[pos].value, sizeof(STG_PAIR::value));
        strncpy(pairs[pos].key, source[pos].first.c_str(), sizeof(STG_PAIR::key));
        strncpy(pairs[pos].value, source[pos].second.c_str(), sizeof(STG_PAIR::value));
        ++pos;
    }
    bzero(pairs[sources.size()].key, sizeof(STG_PAIR::key));
    bzero(pairs[sources.size()].value, sizeof(STG_PAIR::value));

    return pairs;
}
