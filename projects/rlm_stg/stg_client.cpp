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
#include <cstring>

#include "stg_client.h"

using namespace std;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
STG_CLIENT::STG_CLIENT()
    : port(0),
      localPort(0),
      sock(0)
{
}
//-----------------------------------------------------------------------------
STG_CLIENT::~STG_CLIENT()
{
}
//-----------------------------------------------------------------------------
void STG_CLIENT::SetServer(const string & host)
{
STG_CLIENT::host = host;
}
//-----------------------------------------------------------------------------
void STG_CLIENT::SetPort(uint16_t port)
{
STG_CLIENT::port = port;
}
//-----------------------------------------------------------------------------
void STG_CLIENT::SetLocalPort(uint16_t port)
{
STG_CLIENT::localPort = port;
}
//-----------------------------------------------------------------------------
void STG_CLIENT::SetPassword(const string & password)
{
STG_CLIENT::password = password;
}
//-----------------------------------------------------------------------------
uint32_t STG_CLIENT::GetFramedIP() const
{
return framedIP;
}
//-----------------------------------------------------------------------------
void STG_CLIENT::InitEncrypt()
{
unsigned char keyL[RAD_PASSWORD_LEN];
memset(keyL, 0, RAD_PASSWORD_LEN);
strncpy((char *)keyL, password.c_str(), RAD_PASSWORD_LEN);
Blowfish_Init(&ctx, keyL, RAD_PASSWORD_LEN);
}
//-----------------------------------------------------------------------------
int STG_CLIENT::PrepareNet()
{
sock = socket(AF_INET, SOCK_DGRAM, 0);
if (sock == -1)
    {
    errorStr = "Socket create error";
    return -1;
    }

struct hostent * he = NULL;
he = gethostbyname(host.c_str());
if (he == NULL)
    {
    errorStr = "gethostbyname error";
    return -1;
    }

if (localPort != 0)
    {
    struct sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(localPort);
    localAddr.sin_addr.s_addr = inet_addr("0.0.0.0");;

    if (bind(sock, (struct sockaddr *)&localAddr, sizeof(localAddr)))
        {
        errorStr = "Bind failed";
        return -1;
        }
    }

outerAddr.sin_family = AF_INET;
outerAddr.sin_port = htons(port);
outerAddr.sin_addr.s_addr = *(uint32_t *)he->h_addr;

outerAddrLen = sizeof(struct sockaddr_in);

return 0;
}
//-----------------------------------------------------------------------------
void STG_CLIENT::FinalizeNet()
{
close(sock);
}
//-----------------------------------------------------------------------------
int STG_CLIENT::Start()
{
InitEncrypt();

return PrepareNet();
}
//-----------------------------------------------------------------------------
int STG_CLIENT::Stop()
{
FinalizeNet();

return 0;
}
//-----------------------------------------------------------------------------
string STG_CLIENT::GetUserPassword() const
{
return userPassword;
}
//-----------------------------------------------------------------------------
int STG_CLIENT::Send(const RAD_PACKET & packet)
{
char buf[RAD_MAX_PACKET_LEN];
    
Encrypt(buf, (char *)&packet, sizeof(RAD_PACKET) / 8);

int res = sendto(sock, buf, sizeof(RAD_PACKET), 0, (struct sockaddr *)&outerAddr, outerAddrLen);

if (res == -1)
    errorStr = "Error sending data";

return res;
}
//-----------------------------------------------------------------------------
int STG_CLIENT::RecvData(RAD_PACKET * packet)
{
char buf[RAD_MAX_PACKET_LEN];
int res;

outerAddrLen = sizeof(struct sockaddr_in);

res = recvfrom(sock, buf, RAD_MAX_PACKET_LEN, 0, (struct sockaddr *)&outerAddr, &outerAddrLen);
if (res == -1)
    {
    errorStr = "Error receiving data";
    return -1;
    }

Decrypt((char *)packet, buf, res / 8);

return 0;
}
//-----------------------------------------------------------------------------
int STG_CLIENT::Request(RAD_PACKET * packet, const std::string & login, const std::string & svc, uint8_t packetType)
{
int res;

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

return 0;
}
//-----------------------------------------------------------------------------
int STG_CLIENT::Authorize(const string & login, const string & svc)
{
RAD_PACKET packet;

userPassword = "";

if (Request(&packet, login, svc, RAD_AUTZ_PACKET))
    return -1;

if (packet.packetType != RAD_ACCEPT_PACKET)
    return -1;

userPassword = (char *)packet.password;

return 0;
}
//-----------------------------------------------------------------------------
int STG_CLIENT::Authenticate(const string & login, const string & svc)
{
RAD_PACKET packet;

userPassword = "";

if (Request(&packet, login, svc, RAD_AUTH_PACKET))
    return -1;

if (packet.packetType != RAD_ACCEPT_PACKET)
    return -1;

return 0;
}
//-----------------------------------------------------------------------------
int STG_CLIENT::PostAuthenticate(const string & login, const string & svc)
{
RAD_PACKET packet;

userPassword = "";

if (Request(&packet, login, svc, RAD_POST_AUTH_PACKET))
    return -1;

if (packet.packetType != RAD_ACCEPT_PACKET)
    return -1;

if (svc == "Framed-User")
    framedIP = packet.ip;
else
    framedIP = 0;

return 0;
}
//-----------------------------------------------------------------------------
int STG_CLIENT::Account(const std::string & type, const string & login, const string & svc, const string & sessid)
{
RAD_PACKET packet;

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
    return -1;

return 0;
}
//-----------------------------------------------------------------------------
void STG_CLIENT::Encrypt(char * dst, const char * src, int len8)         
{
// len8 - длина в 8-ми байтовых блоках                                                   
if (dst != src) 
    memcpy(dst, src, len8 * 8);                                                          
    
for (int i = 0; i < len8; i++)
    Blowfish_Encrypt(&ctx, (uint32_t *)(dst + i*8), (uint32_t *)(dst + i*8 + 4));
}
//-----------------------------------------------------------------------------          
void STG_CLIENT::Decrypt(char * dst, const char * src, int len8)
{
// len8 - длина в 8-ми байтовых блоках
if (dst != src)
    memcpy(dst, src, len8 * 8);

for (int i = 0; i < len8; i++)
    Blowfish_Decrypt(&ctx, (uint32_t *)(dst + i*8), (uint32_t *)(dst + i*8 + 4));
}
//-----------------------------------------------------------------------------
