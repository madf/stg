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
 *  This file contains a realization of radius data access plugin for Stargazer
 *
 *  $Revision: 1.14 $
 *  $Date: 2009/12/13 14:17:13 $
 *
 */

#include <csignal>
#include <cerrno>
#include <algorithm>

#include "stg/store.h"
#include "stg/common.h"
#include "stg/user_conf.h"
#include "stg/user_property.h"
#include "stg/plugin_creator.h"
#include "radius.h"

extern volatile const time_t stgTime;

void InitEncrypt(BLOWFISH_CTX * ctx, const std::string & password);
void Decrypt(BLOWFISH_CTX * ctx, char * dst, const char * src, int len8);
void Encrypt(BLOWFISH_CTX * ctx, char * dst, const char * src, int len8);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN_CREATOR<RADIUS> radc;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN * GetPlugin()
{
return radc.GetPlugin();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int RAD_SETTINGS::ParseServices(const std::vector<std::string> & str, std::list<std::string> * lst)
{
std::copy(str.begin(), str.end(), std::back_inserter(*lst));
std::list<std::string>::iterator it(std::find(lst->begin(),
                               lst->end(),
                               "empty"));
if (it != lst->end())
    *it = "";

return 0;
}
//-----------------------------------------------------------------------------
int RAD_SETTINGS::ParseSettings(const MODULE_SETTINGS & s)
{
int p;
PARAM_VALUE pv;
std::vector<PARAM_VALUE>::const_iterator pvi;
///////////////////////////
pv.param = "Port";
pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end())
    {
    errorStr = "Parameter \'Port\' not found.";
    printfd(__FILE__, "Parameter 'Port' not found\n");
    return -1;
    }
if (ParseIntInRange(pvi->value[0], 2, 65535, &p))
    {
    errorStr = "Cannot parse parameter \'Port\': " + errorStr;
    printfd(__FILE__, "Cannot parse parameter 'Port'\n");
    return -1;
    }
port = p;
///////////////////////////
pv.param = "Password";
pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end())
    {
    errorStr = "Parameter \'Password\' not found.";
    printfd(__FILE__, "Parameter 'Password' not found\n");
    return -1;
    }
password = pvi->value[0];
///////////////////////////
pv.param = "AuthServices";
pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi != s.moduleParams.end())
    {
    ParseServices(pvi->value, &authServices);
    }
///////////////////////////
pv.param = "AcctServices";
pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi != s.moduleParams.end())
    {
    ParseServices(pvi->value, &acctServices);
    }

return 0;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
RADIUS::RADIUS()
    : ctx(),
      errorStr(),
      radSettings(),
      settings(),
      authServices(),
      acctServices(),
      sessions(),
      nonstop(false),
      isRunning(false),
      users(NULL),
      stgSettings(NULL),
      store(NULL),
      thread(),
      mutex(),
      sock(-1),
      packet(),
      logger(GetPluginLogger(GetStgLogger(), "radius"))
{
InitEncrypt(&ctx, "");
}
//-----------------------------------------------------------------------------
int RADIUS::ParseSettings()
{
int ret = radSettings.ParseSettings(settings);
if (ret)
    errorStr = radSettings.GetStrError();
return ret;
}
//-----------------------------------------------------------------------------
int RADIUS::PrepareNet()
{
sock = socket(AF_INET, SOCK_DGRAM, 0);

if (sock < 0)
    {
    errorStr = "Cannot create socket.";
    logger("Cannot create a socket: %s", strerror(errno));
    printfd(__FILE__, "Cannot create socket\n");
    return -1;
    }

struct sockaddr_in inAddr;
inAddr.sin_family = AF_INET;
inAddr.sin_port = htons(radSettings.GetPort());
inAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

if (bind(sock, (struct sockaddr*)&inAddr, sizeof(inAddr)) < 0)
    {
    errorStr = "RADIUS: Bind failed.";
    logger("Cannot bind the socket: %s", strerror(errno));
    printfd(__FILE__, "Cannot bind socket\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int RADIUS::FinalizeNet()
{
close(sock);
return 0;
}
//-----------------------------------------------------------------------------
int RADIUS::Start()
{
std::string password(radSettings.GetPassword());

authServices = radSettings.GetAuthServices();
acctServices = radSettings.GetAcctServices();

InitEncrypt(&ctx, password);

nonstop = true;

if (PrepareNet())
    {
    return -1;
    }

if (!isRunning)
    {
    if (pthread_create(&thread, NULL, Run, this))
        {
        errorStr = "Cannot create thread.";
	logger("Cannot create thread.");
        printfd(__FILE__, "Cannot create thread\n");
        return -1;
        }
    }

errorStr = "";
return 0;
}
//-----------------------------------------------------------------------------
int RADIUS::Stop()
{
if (!IsRunning())
    return 0;

nonstop = false;

std::map<std::string, RAD_SESSION>::iterator it;
for (it = sessions.begin(); it != sessions.end(); ++it)
    {
    USER_PTR ui;
    if (users->FindByName(it->second.userName, &ui))
        {
        users->Unauthorize(ui->GetLogin(), this);
        }
    }
sessions.erase(sessions.begin(), sessions.end());

FinalizeNet();

if (isRunning)
    {
    //5 seconds to thread stops itself
    for (int i = 0; i < 25 && isRunning; i++)
        {
        struct timespec ts = {0, 200000000};
        nanosleep(&ts, NULL);
        }
    }

if (isRunning)
    return -1;

return 0;
}
//-----------------------------------------------------------------------------
void * RADIUS::Run(void * d)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

RADIUS * rad = (RADIUS *)d;
RAD_PACKET packet;

rad->isRunning = true;

while (rad->nonstop)
    {
    if (!WaitPackets(rad->sock))
        {
        continue;
        }
    struct sockaddr_in outerAddr;
    if (rad->RecvData(&packet, &outerAddr))
        {
        printfd(__FILE__, "RADIUS::Run Error on RecvData\n");
        }
    else
        {
        if (rad->ProcessData(&packet))
            {
            packet.packetType = RAD_REJECT_PACKET;
            }
        rad->Send(packet, &outerAddr);
        }
    }

rad->isRunning = false;

return NULL;
}
//-----------------------------------------------------------------------------
int RADIUS::RecvData(RAD_PACKET * packet, struct sockaddr_in * outerAddr)
{
    int8_t buf[RAD_MAX_PACKET_LEN];
    socklen_t outerAddrLen = sizeof(struct sockaddr_in);
    int dataLen = recvfrom(sock, buf, RAD_MAX_PACKET_LEN, 0, reinterpret_cast<struct sockaddr *>(outerAddr), &outerAddrLen);
    if (dataLen < 0)
    	{
	logger("recvfrom error: %s", strerror(errno));
	return -1;
	}
    if (dataLen == 0)
    	return -1;

    Decrypt(&ctx, (char *)packet, (const char *)buf, dataLen / 8);

    if (strncmp((char *)packet->magic, RAD_ID, RAD_MAGIC_LEN))
        {
        printfd(__FILE__, "RADIUS::RecvData Error magic. Wanted: '%s', got: '%s'\n", RAD_ID, packet->magic);
        return -1;
        }

    return 0;
}
//-----------------------------------------------------------------------------
int RADIUS::Send(const RAD_PACKET & packet, struct sockaddr_in * outerAddr)
{
size_t len = sizeof(RAD_PACKET);
char buf[1032];

Encrypt(&ctx, buf, (char *)&packet, len / 8);
int res = sendto(sock, buf, len, 0, reinterpret_cast<struct sockaddr *>(outerAddr), sizeof(struct sockaddr_in));
if (res < 0)
    logger("sendto error: %s", strerror(errno));
return res;
}
//-----------------------------------------------------------------------------
int RADIUS::ProcessData(RAD_PACKET * packet)
{
if (strncmp((const char *)packet->protoVer, "01", 2))
    {
    printfd(__FILE__, "RADIUS::ProcessData packet.protoVer incorrect\n");
    return -1;
    }
switch (packet->packetType)
    {
    case RAD_AUTZ_PACKET:
        return ProcessAutzPacket(packet);
    case RAD_AUTH_PACKET:
        return ProcessAuthPacket(packet);
    case RAD_POST_AUTH_PACKET:
        return ProcessPostAuthPacket(packet);
    case RAD_ACCT_START_PACKET:
        return ProcessAcctStartPacket(packet);
    case RAD_ACCT_STOP_PACKET:
        return ProcessAcctStopPacket(packet);
    case RAD_ACCT_UPDATE_PACKET:
        return ProcessAcctUpdatePacket(packet);
    case RAD_ACCT_OTHER_PACKET:
        return ProcessAcctOtherPacket(packet);
    default:
        printfd(__FILE__, "RADIUS::ProcessData Unsupported packet type: %d\n", packet->packetType);
        return -1;
    };
return 0;
}
//-----------------------------------------------------------------------------
int RADIUS::ProcessAutzPacket(RAD_PACKET * packet)
{
USER_CONF conf;

if (!IsAllowedService((char *)packet->service))
    {
    printfd(__FILE__, "RADIUS::ProcessAutzPacket service '%s' is not allowed to authorize\n", packet->service);
    packet->packetType = RAD_REJECT_PACKET;
    return 0;
    }

if (store->RestoreUserConf(&conf, (char *)packet->login))
    {
    packet->packetType = RAD_REJECT_PACKET;
    printfd(__FILE__, "RADIUS::ProcessAutzPacket cannot restore conf for user '%s'\n", packet->login);
    return 0;
    }

// At this point service can be authorized at least
// So we send a plain-text password

packet->packetType = RAD_ACCEPT_PACKET;
strncpy((char *)packet->password, conf.password.c_str(), RAD_PASSWORD_LEN);

return 0;
}
//-----------------------------------------------------------------------------
int RADIUS::ProcessAuthPacket(RAD_PACKET * packet)
{
USER_PTR ui;

if (!CanAcctService((char *)packet->service))
    {

    // There are no sense to check for allowed service
    // It has allready checked at previous stage (authorization)

    printfd(__FILE__, "RADIUS::ProcessAuthPacket service '%s' neednot stargazer authentication\n", (char *)packet->service);
    packet->packetType = RAD_ACCEPT_PACKET;
    return 0;
    }

// At this point we have an accountable service
// All other services got a password if allowed or rejected

if (!FindUser(&ui, (char *)packet->login))
    {
    packet->packetType = RAD_REJECT_PACKET;
    printfd(__FILE__, "RADIUS::ProcessAuthPacket user '%s' not found\n", (char *)packet->login);
    return 0;
    }

if (ui->IsInetable())
    {
    packet->packetType = RAD_ACCEPT_PACKET;
    }
else
    {
    packet->packetType = RAD_REJECT_PACKET;
    }

packet->packetType = RAD_ACCEPT_PACKET;
return 0;
}
//-----------------------------------------------------------------------------
int RADIUS::ProcessPostAuthPacket(RAD_PACKET * packet)
{
USER_PTR ui;

if (!CanAcctService((char *)packet->service))
    {

    // There are no sense to check for allowed service
    // It has allready checked at previous stage (authorization)

    packet->packetType = RAD_ACCEPT_PACKET;
    return 0;
    }

if (!FindUser(&ui, (char *)packet->login))
    {
    packet->packetType = RAD_REJECT_PACKET;
    printfd(__FILE__, "RADIUS::ProcessPostAuthPacket user '%s' not found\n", (char *)packet->login);
    return 0;
    }

// I think that only Framed-User services has sense to be accountable
// So we have to supply a Framed-IP

USER_IPS ips = ui->GetProperty().ips;
packet->packetType = RAD_ACCEPT_PACKET;

// Additional checking for Framed-User service

if (!strncmp((char *)packet->service, "Framed-User", RAD_SERVICE_LEN))
    packet->ip = ips[0].ip;
else
    packet->ip = 0;

return 0;
}
//-----------------------------------------------------------------------------
int RADIUS::ProcessAcctStartPacket(RAD_PACKET * packet)
{
USER_PTR ui;

if (!FindUser(&ui, (char *)packet->login))
    {
    packet->packetType = RAD_REJECT_PACKET;
    printfd(__FILE__, "RADIUS::ProcessAcctStartPacket user '%s' not found\n", (char *)packet->login);
    return 0;
    }

// At this point we have to unauthorize user only if it is an accountable service

if (CanAcctService((char *)packet->service))
    {
    if (sessions.find((const char *)packet->sessid) != sessions.end())
        {
        printfd(__FILE__, "RADIUS::ProcessAcctStartPacket session already started!\n");
        packet->packetType = RAD_REJECT_PACKET;
        return -1;
        }
    USER_IPS ips = ui->GetProperty().ips;
    if (!users->Authorize(ui->GetLogin(), ips[0].ip, 0xffFFffFF, this))
        {
        printfd(__FILE__, "RADIUS::ProcessAcctStartPacket cannot authorize user '%s'\n", packet->login);
        packet->packetType = RAD_REJECT_PACKET;
        return -1;
        }
    sessions[(const char *)packet->sessid].userName = (const char *)packet->login;
    sessions[(const char *)packet->sessid].serviceType = (const char *)packet->service;
    for_each(sessions.begin(), sessions.end(), SPrinter());
    }
else
    {
    printfd(__FILE__, "RADIUS::ProcessAcctStartPacket service '%s' can not be accounted\n", (char *)packet->service);
    }

packet->packetType = RAD_ACCEPT_PACKET;
return 0;
}
//-----------------------------------------------------------------------------
int RADIUS::ProcessAcctStopPacket(RAD_PACKET * packet)
{
std::map<std::string, RAD_SESSION>::iterator sid;

if ((sid = sessions.find((const char *)packet->sessid)) == sessions.end())
    {
    printfd(__FILE__, "RADIUS::ProcessAcctStopPacket session had not started yet\n");
    packet->packetType = RAD_REJECT_PACKET;
    return -1;
    }

USER_PTR ui;

if (!FindUser(&ui, sid->second.userName))
    {
    packet->packetType = RAD_REJECT_PACKET;
    printfd(__FILE__, "RADIUS::ProcessPostAuthPacket user '%s' not found\n", sid->second.userName.c_str());
    return 0;
    }

sessions.erase(sid);

users->Unauthorize(ui->GetLogin(), this);

packet->packetType = RAD_ACCEPT_PACKET;
return 0;
}
//-----------------------------------------------------------------------------
int RADIUS::ProcessAcctUpdatePacket(RAD_PACKET * packet)
{
// Fake. May be use it later
packet->packetType = RAD_ACCEPT_PACKET;
return 0;
}
//-----------------------------------------------------------------------------
int RADIUS::ProcessAcctOtherPacket(RAD_PACKET * packet)
{
// Fake. May be use it later
packet->packetType = RAD_ACCEPT_PACKET;
return 0;
}
//-----------------------------------------------------------------------------
void RADIUS::PrintServices(const std::list<std::string> & svcs)
{
for_each(svcs.begin(), svcs.end(), Printer());
}
//-----------------------------------------------------------------------------
bool RADIUS::FindUser(USER_PTR * ui, const std::string & login) const
{
if (users->FindByName(login, ui))
    {
    return false;
    }
return true;
}
//-----------------------------------------------------------------------------
bool RADIUS::CanAuthService(const std::string & svc) const
{
return find(authServices.begin(), authServices.end(), svc) != authServices.end();
}
//-----------------------------------------------------------------------------
bool RADIUS::CanAcctService(const std::string & svc) const
{
return find(acctServices.begin(), acctServices.end(), svc) != acctServices.end();
}
//-----------------------------------------------------------------------------
bool RADIUS::IsAllowedService(const std::string & svc) const
{
return CanAuthService(svc) || CanAcctService(svc);
}
//-----------------------------------------------------------------------------
inline
void InitEncrypt(BLOWFISH_CTX * ctx, const std::string & password)
{
unsigned char keyL[RAD_PASSWORD_LEN];  // Пароль для шифровки
memset(keyL, 0, RAD_PASSWORD_LEN);
strncpy((char *)keyL, password.c_str(), RAD_PASSWORD_LEN);
Blowfish_Init(ctx, keyL, RAD_PASSWORD_LEN);
}
//-----------------------------------------------------------------------------
inline
void Encrypt(BLOWFISH_CTX * ctx, char * dst, const char * src, int len8)
{
// len8 - длина в 8-ми байтовых блоках
if (dst != src)
    memcpy(dst, src, len8 * 8);

for (int i = 0; i < len8; i++)
    Blowfish_Encrypt(ctx, (uint32_t *)(dst + i*8), (uint32_t *)(dst + i*8 + 4));
}
//-----------------------------------------------------------------------------
inline
void Decrypt(BLOWFISH_CTX * ctx, char * dst, const char * src, int len8)
{
// len8 - длина в 8-ми байтовых блоках
if (dst != src)
    memcpy(dst, src, len8 * 8);

for (int i = 0; i < len8; i++)
    Blowfish_Decrypt(ctx, (uint32_t *)(dst + i*8), (uint32_t *)(dst + i*8 + 4));
}
