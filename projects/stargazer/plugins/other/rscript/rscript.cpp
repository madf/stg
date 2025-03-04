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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#include "rscript.h"

#include "ur_functor.h"

#include "stg/common.h"
#include "stg/users.h"
#include "stg/user_property.h"
#include "stg/logger.h"

#include <algorithm>

#include <csignal>
#include <cassert>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <sys/time.h>
#include <netinet/ip.h>

#define RS_DEBUG (1)
#define MAX_SHORT_PCKT  (3)

extern volatile time_t stgTime;

namespace RS = STG::RS;
using RS::REMOTE_SCRIPT;

extern "C" STG::Plugin* GetPlugin()
{
    static REMOTE_SCRIPT plugin;
    return &plugin;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
RS::SETTINGS::SETTINGS()
    : sendPeriod(0),
      port(0)
{
}
//-----------------------------------------------------------------------------
int RS::SETTINGS::ParseSettings(const ModuleSettings & s)
{
int p;
ParamValue pv;
netRouters.clear();
///////////////////////////
pv.param = "Port";
auto pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
    errorStr = "Parameter \'Port\' not found.";
    printfd(__FILE__, "Parameter 'Port' not found\n");
    return -1;
    }
if (ParseIntInRange(pvi->value[0], 2, 65535, &p) != 0)
    {
    errorStr = "Cannot parse parameter \'Port\': " + errorStr;
    printfd(__FILE__, "Cannot parse parameter 'Port'\n");
    return -1;
    }
port = static_cast<uint16_t>(p);
///////////////////////////
pv.param = "SendPeriod";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
    errorStr = "Parameter \'SendPeriod\' not found.";
    printfd(__FILE__, "Parameter 'SendPeriod' not found\n");
    return -1;
    }

if (ParseIntInRange(pvi->value[0], 5, 600, &sendPeriod) != 0)
    {
    errorStr = "Cannot parse parameter \'SendPeriod\': " + errorStr;
    printfd(__FILE__, "Cannot parse parameter 'SendPeriod'\n");
    return -1;
    }
///////////////////////////
pv.param = "UserParams";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
    errorStr = "Parameter \'UserParams\' not found.";
    printfd(__FILE__, "Parameter 'UserParams' not found\n");
    return -1;
    }
userParams = pvi->value;
///////////////////////////
pv.param = "Password";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
    errorStr = "Parameter \'Password\' not found.";
    printfd(__FILE__, "Parameter 'Password' not found\n");
    return -1;
    }
password = pvi->value[0];
///////////////////////////
pv.param = "SubnetFile";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
    errorStr = "Parameter \'SubnetFile\' not found.";
    printfd(__FILE__, "Parameter 'SubnetFile' not found\n");
    return -1;
    }
subnetFile = pvi->value[0];

NRMapParser nrMapParser;

if (!nrMapParser.ReadFile(subnetFile))
    netRouters = nrMapParser.GetMap();
else
    PluginLogger::get("rscript")("mod_rscript: error opening subnets file '%s'", subnetFile.c_str());

return 0;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
REMOTE_SCRIPT::REMOTE_SCRIPT()
    : sendPeriod(15),
      halfPeriod(8),
      isRunning(false),
      users(nullptr),
      sock(0),
      logger(PluginLogger::get("rscript"))
{
}
//-----------------------------------------------------------------------------
void REMOTE_SCRIPT::Run(std::stop_token token)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, nullptr);

isRunning = true;

while (!token.stop_requested())
    {
    PeriodicSend();
    sleep(2);
    }

isRunning = false;
}
//-----------------------------------------------------------------------------
int REMOTE_SCRIPT::ParseSettings()
{
auto ret = rsSettings.ParseSettings(settings);
if (ret != 0)
    errorStr = rsSettings.GetStrError();

sendPeriod = rsSettings.GetSendPeriod();
halfPeriod = sendPeriod / 2;

return ret;
}
//-----------------------------------------------------------------------------
int REMOTE_SCRIPT::Start()
{
netRouters = rsSettings.GetSubnetsMap();

InitEncrypt(rsSettings.GetPassword());

m_onAddUserConn = users->onAdd([this](auto user){ AddUser(user); });
m_onDelUserConn = users->onDel([this](auto user){ DelUser(user); });

if (GetUsers())
    return -1;

if (PrepareNet())
    return -1;

if (!isRunning)
    m_thread = std::jthread([this](auto token){ Run(std::move(token)); });

errorStr = "";
return 0;
}
//-----------------------------------------------------------------------------
int REMOTE_SCRIPT::Stop()
{
if (!IsRunning())
    return 0;

m_thread.request_stop();

std::for_each(
    authorizedUsers.begin(),
    authorizedUsers.end(),
    [this](auto& kv){ Send(kv.second, true); }
);

FinalizeNet();

if (isRunning)
    {
    //5 seconds to thread stops itself
    for (int i = 0; i < 25 && isRunning; i++)
        {
        struct timespec ts = {0, 200000000};
        nanosleep(&ts, nullptr);
        }
    }

if (isRunning)
    {
    logger("Cannot stop thread.");
    m_thread.detach();
    }
else
    m_thread.join();

return 0;
}
//-----------------------------------------------------------------------------
int REMOTE_SCRIPT::Reload(const ModuleSettings & /*ms*/)
{
NRMapParser nrMapParser;

if (nrMapParser.ReadFile(rsSettings.GetMapFileName()))
    {
    errorStr = nrMapParser.GetErrorStr();
    logger("Map file reading error: %s", errorStr.c_str());
    return -1;
    }

    {
    std::lock_guard lock(m_mutex);

    printfd(__FILE__, "REMOTE_SCRIPT::Reload()\n");

    netRouters = nrMapParser.GetMap();
    }

std::for_each(authorizedUsers.begin(),
              authorizedUsers.end(),
              UpdateRouter(*this));

logger("%s reloaded successfully.", rsSettings.GetMapFileName().c_str());
printfd(__FILE__, "REMOTE_SCRIPT::Reload() %s reloaded successfully.\n");

return 0;
}
//-----------------------------------------------------------------------------
bool REMOTE_SCRIPT::PrepareNet()
{
sock = socket(AF_INET, SOCK_DGRAM, 0);

if (sock < 0)
    {
    errorStr = "Cannot create socket.";
    logger("Canot create a socket: %s", strerror(errno));
    printfd(__FILE__, "Cannot create socket\n");
    return true;
    }

return false;
}
//-----------------------------------------------------------------------------
bool REMOTE_SCRIPT::FinalizeNet()
{
close(sock);
return false;
}
//-----------------------------------------------------------------------------
void REMOTE_SCRIPT::PeriodicSend()
{
std::lock_guard lock(m_mutex);

auto it = authorizedUsers.begin();
while (it != authorizedUsers.end())
    {
    if (difftime(stgTime, it->second.lastSentTime) - (rand() % halfPeriod) > sendPeriod)
        {
        Send(it->second);
        }
    ++it;
    }
}
//-----------------------------------------------------------------------------
#ifdef NDEBUG
bool REMOTE_SCRIPT::PreparePacket(char * buf, size_t, RS::USER & rsu, bool forceDisconnect) const
#else
bool REMOTE_SCRIPT::PreparePacket(char * buf, size_t bufSize, RS::USER & rsu, bool forceDisconnect) const
#endif
{
RS::PACKET_HEADER packetHead;

memset(packetHead.padding, 0, sizeof(packetHead.padding));
memcpy(packetHead.magic, RS_ID, sizeof(RS_ID));
packetHead.protoVer[0] = '0';
packetHead.protoVer[1] = '2';
if (forceDisconnect)
    {
    packetHead.packetType = RS_DISCONNECT_PACKET;
    printfd(__FILE__, "RSCRIPT: force disconnect for '%s'\n", rsu.user->GetLogin().c_str());
    }
else
    {
    if (rsu.shortPacketsCount % MAX_SHORT_PCKT == 0)
        {
        //SendLong
        packetHead.packetType = rsu.user->IsInetable() ? RS_CONNECT_PACKET : RS_DISCONNECT_PACKET;
        if (rsu.user->IsInetable())
            printfd(__FILE__, "RSCRIPT: connect for '%s'\n", rsu.user->GetLogin().c_str());
        else
            printfd(__FILE__, "RSCRIPT: disconnect for '%s'\n", rsu.user->GetLogin().c_str());
        }
    else
        {
        //SendShort
        packetHead.packetType = rsu.user->IsInetable() ? RS_ALIVE_PACKET : RS_DISCONNECT_PACKET;
        if (rsu.user->IsInetable())
            printfd(__FILE__, "RSCRIPT: alive for '%s'\n", rsu.user->GetLogin().c_str());
        else
            printfd(__FILE__, "RSCRIPT: disconnect for '%s'\n", rsu.user->GetLogin().c_str());
        }
    }
rsu.shortPacketsCount++;
rsu.lastSentTime = stgTime;

packetHead.ip = htonl(rsu.ip);
packetHead.id = htonl(rsu.user->GetID());
strncpy(reinterpret_cast<char*>(packetHead.login), rsu.user->GetLogin().c_str(), RS_LOGIN_LEN);
packetHead.login[RS_LOGIN_LEN - 1] = 0;

memcpy(buf, &packetHead, sizeof(packetHead));

if (packetHead.packetType == RS_ALIVE_PACKET)
    {
    return false;
    }

RS::PACKET_TAIL packetTail;

memset(packetTail.padding, 0, sizeof(packetTail.padding));
memcpy(packetTail.magic, RS_ID, sizeof(RS_ID));
std::string params;
for (const auto& param : rsSettings.GetUserParams())
    {
    auto value = rsu.user->GetParamValue(param);
    if (params.length() + value.length() > RS_PARAMS_LEN - 1)
    {
        logger("Script params string length %d exceeds the limit of %d symbols.", params.length() + value.length(), RS_PARAMS_LEN);
        break;
    }
    params += value + " ";
    }
strncpy(reinterpret_cast<char*>(packetTail.params), params.c_str(), RS_PARAMS_LEN);
packetTail.params[RS_PARAMS_LEN - 1] = 0;

assert(sizeof(packetHead) + sizeof(packetTail) <= bufSize && "Insufficient buffer space");

Encrypt(buf + sizeof(packetHead), reinterpret_cast<char *>(&packetTail), sizeof(packetTail) / 8);

return false;
}
//-----------------------------------------------------------------------------
bool REMOTE_SCRIPT::Send(RS::USER & rsu, bool forceDisconnect) const
{
char buffer[RS_MAX_PACKET_LEN];

memset(buffer, 0, sizeof(buffer));

if (PreparePacket(buffer, sizeof(buffer), rsu, forceDisconnect))
    {
    printfd(__FILE__, "REMOTE_SCRIPT::Send() - Invalid packet length!\n");
    return true;
    }

for (const auto& ip : rsu.routers)
{
    struct sockaddr_in sendAddr;

    sendAddr.sin_family = AF_INET;
    sendAddr.sin_port = htons(rsSettings.GetPort());
    sendAddr.sin_addr.s_addr = ip;

    return sendto(sock, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr*>(&sendAddr), sizeof(sendAddr)) > 0;
}

return false;
}
//-----------------------------------------------------------------------------
bool REMOTE_SCRIPT::SendDirect(RS::USER & rsu, uint32_t routerIP, bool forceDisconnect) const
{
char buffer[RS_MAX_PACKET_LEN];

if (PreparePacket(buffer, sizeof(buffer), rsu, forceDisconnect))
    {
    printfd(__FILE__, "REMOTE_SCRIPT::SendDirect() - Invalid packet length!\n");
    return true;
    }

struct sockaddr_in sendAddr;

sendAddr.sin_family = AF_INET;
sendAddr.sin_port = htons(rsSettings.GetPort());
sendAddr.sin_addr.s_addr = routerIP;

ssize_t res = sendto(sock, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&sendAddr), sizeof(sendAddr));

if (res < 0)
    logger("sendto error: %s", strerror(errno));

return (res != sizeof(buffer));
}
//-----------------------------------------------------------------------------
bool REMOTE_SCRIPT::GetUsers()
{
UserPtr u;

int h = users->OpenSearch();
assert(h && "USERS::OpenSearch is always correct");

while (users->SearchNext(h, &u) != 0)
    SetUserNotifiers(u);

users->CloseSearch(h);
return false;
}
//-----------------------------------------------------------------------------
std::vector<uint32_t> REMOTE_SCRIPT::IP2Routers(uint32_t ip)
{
std::lock_guard lock(m_mutex);
for (auto& nr : netRouters)
    if ((ip & nr.subnetMask) == (nr.subnetIP & nr.subnetMask))
        return nr.routers;
return {};
}
//-----------------------------------------------------------------------------
void REMOTE_SCRIPT::SetUserNotifiers(UserPtr u)
{
    m_conns.emplace_back(
        u->GetID(),
        u->afterCurrIPChange([this, u](auto, auto newVal){ addDelUser(u, newVal != 0); }),
        u->afterConnectedChange([this, u](auto, auto newVal){ addDelUser(u, newVal); })
    );
}
//-----------------------------------------------------------------------------
void REMOTE_SCRIPT::UnSetUserNotifiers(UserPtr u)
{
    m_conns.erase(std::remove_if(m_conns.begin(), m_conns.end(),
                  [u](const auto& c){ return std::get<0>(c) == u->GetID(); }),
                  m_conns.end());

}
//-----------------------------------------------------------------------------
void REMOTE_SCRIPT::AddRSU(UserPtr user)
{
RS::USER rsu(IP2Routers(user->GetCurrIP()), user);
Send(rsu);

std::lock_guard lock(m_mutex);
authorizedUsers.insert(std::make_pair(user->GetCurrIP(), rsu));
}
//-----------------------------------------------------------------------------
void REMOTE_SCRIPT::DelRSU(UserPtr user)
{
std::lock_guard lock(m_mutex);
auto it = authorizedUsers.begin();
while (it != authorizedUsers.end())
    {
    if (it->second.user == user)
        {
        Send(it->second, true);
        authorizedUsers.erase(it);
        return;
        }
    ++it;
    }
/*const auto it = authorizedUsers.find(user->GetCurrIP());
if (it != authorizedUsers.end())
    {
    Send(it->second, true);
    authorizedUsers.erase(it);
    }*/
}
//-----------------------------------------------------------------------------
void REMOTE_SCRIPT::addDelUser(UserPtr user, bool toAdd)
{
    if (toAdd)
        AddRSU(user);
    else
        DelRSU(user);
}
//-----------------------------------------------------------------------------
void REMOTE_SCRIPT::InitEncrypt(const std::string & password) const
{
unsigned char keyL[PASSWD_LEN];  // Пароль для шифровки
memset(keyL, 0, PASSWD_LEN);
strncpy(reinterpret_cast<char*>(keyL), password.c_str(), PASSWD_LEN);
Blowfish_Init(&ctx, keyL, PASSWD_LEN);
}
//-----------------------------------------------------------------------------
void REMOTE_SCRIPT::Encrypt(void * dst, const void * src, size_t len8) const
{
if (dst != src)
    memcpy(dst, src, len8 * 8);
for (size_t i = 0; i < len8; ++i)
    Blowfish_Encrypt(&ctx, static_cast<uint32_t *>(dst) + i * 2, static_cast<uint32_t *>(dst) + i * 2 + 1);
}
//-----------------------------------------------------------------------------
