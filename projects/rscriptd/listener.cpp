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

#include "listener.h"

#include "stg/scriptexecuter.h"
#include "stg/locker.h"
#include "stg/common.h"
#include "stg/const.h"

#include <sstream>
#include <algorithm>
#include <chrono>
#include <csignal>
#include <cerrno>
#include <ctime>
#include <cstring>

#include <arpa/inet.h>
#include <sys/uio.h> // readv
#include <sys/types.h> // for historical versions of BSD
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

void InitEncrypt(BLOWFISH_CTX * ctx, const std::string & password);

//-----------------------------------------------------------------------------
LISTENER::LISTENER()
    : WriteServLog(STG::Logger::get()),
      port(0),
      receiverStopped(true),
      processorStopped(true),
      userTimeout(0),
      listenSocket(0),
      version("rscriptd listener v.1.2")
{
}
//-----------------------------------------------------------------------------
void LISTENER::SetPassword(const std::string & p)
{
password = p;
printfd(__FILE__, "Encryption initiated with password \'%s\'\n", password.c_str());
InitEncrypt(&ctxS, password);
}
//-----------------------------------------------------------------------------
bool LISTENER::Start()
{
printfd(__FILE__, "LISTENER::Start()\n");

if (PrepareNet())
    {
    return true;
    }

if (!m_receiverThread.joinable())
    m_receiverThread = std::jthread([this](auto token){ Run(std::move(token)); });

if (!m_processorThread.joinable())
    m_processorThread = std::jthread([this](auto token){ RunProcessor(std::move(token)); });

errorStr = "";

return false;
}
//-----------------------------------------------------------------------------
bool LISTENER::Stop()
{
m_receiverThread.request_stop();
m_processorThread.request_stop();

printfd(__FILE__, "LISTENER::Stop()\n");

std::this_thread::sleep_for(std::chrono::milliseconds(500));

if (!processorStopped)
    {
    //5 seconds to thread stops itself
    for (int i = 0; i < 25 && !processorStopped; i++)
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

    //after 5 seconds waiting thread still running. now killing it
    if (!processorStopped)
        {
        //TODO pthread_cancel()
        m_processorThread.detach();
        printfd(__FILE__, "LISTENER killed Timeouter\n");
        }
    }

if (!receiverStopped)
    {
    //5 seconds to thread stops itself
    for (int i = 0; i < 25 && !receiverStopped; i++)
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

    //after 5 seconds waiting thread still running. now killing it
    if (!receiverStopped)
        {
        //TODO pthread_cancel()
        m_receiverThread.detach();
        printfd(__FILE__, "LISTENER killed Run\n");
        }
    }

if (receiverStopped)
    m_receiverThread.join();
if (processorStopped)
    m_processorThread.join();

FinalizeNet();

for (const auto& user : users)
    Disconnect(user);

printfd(__FILE__, "LISTENER::Stoped successfully.\n");

return false;
}
//-----------------------------------------------------------------------------
void LISTENER::Run(std::stop_token token)
{
receiverStopped = false;

while (!token.stop_requested())
    RecvPacket(token);

receiverStopped = true;
}
//-----------------------------------------------------------------------------
void LISTENER::RunProcessor(std::stop_token token)
{
processorStopped = false;

while (!token.stop_requested())
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if (!pending.empty())
        ProcessPending();
    ProcessTimeouts();
}

processorStopped = true;
}
//-----------------------------------------------------------------------------
bool LISTENER::PrepareNet()
{
listenSocket = socket(AF_INET, SOCK_DGRAM, 0);

if (listenSocket < 0)
    {
    errorStr = "Cannot create socket.";
    return true;
    }

struct sockaddr_in listenAddr;
listenAddr.sin_family = AF_INET;
listenAddr.sin_port = htons(port);
listenAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

if (bind(listenSocket, reinterpret_cast<sockaddr*>(&listenAddr), sizeof(listenAddr)) < 0)
    {
    errorStr = "LISTENER: Bind failed.";
    return true;
    }

printfd(__FILE__, "LISTENER::PrepareNet() >>>> Start successfull.\n");

return false;
}
//-----------------------------------------------------------------------------
bool LISTENER::FinalizeNet()
{
close(listenSocket);

return false;
}
//-----------------------------------------------------------------------------
bool LISTENER::RecvPacket(const std::stop_token& token)
{
struct iovec iov[2];

char buffer[RS_MAX_PACKET_LEN];
STG::RS::PACKET_HEADER packetHead;

iov[0].iov_base = reinterpret_cast<char *>(&packetHead);
iov[0].iov_len = sizeof(packetHead);
iov[1].iov_base = buffer;
iov[1].iov_len = sizeof(buffer) - sizeof(packetHead);

size_t dataLen = 0;
while (dataLen < sizeof(buffer))
    {
    if (!WaitPackets(listenSocket))
        {
        if (token.stop_requested())
            return false;
        continue;
        }
    int portion = readv(listenSocket, iov, 2);
    if (portion < 0)
        {
        return true;
        }
    dataLen += portion;
    }

if (CheckHeader(packetHead))
    {
    printfd(__FILE__, "Invalid packet or incorrect protocol version!\n");
    return true;
    }

std::string userLogin(reinterpret_cast<const char*>(packetHead.login));
PendingData pd;
pd.data.login = userLogin;
pd.data.ip = ntohl(packetHead.ip);
pd.data.id = ntohl(packetHead.id);

if (packetHead.packetType == RS_ALIVE_PACKET)
    {
    pd.type = PendingData::ALIVE;
    }
else if (packetHead.packetType == RS_CONNECT_PACKET)
    {
    pd.type = PendingData::CONNECT;
    if (GetParams(buffer, pd.data))
        {
        return true;
        }
    }
else if (packetHead.packetType == RS_DISCONNECT_PACKET)
    {
    pd.type = PendingData::DISCONNECT;
    if (GetParams(buffer, pd.data))
        {
        return true;
        }
    }
else
    return true;

std::lock_guard lock(m_mutex);
pending.push_back(pd);

return false;
}
//-----------------------------------------------------------------------------
bool LISTENER::GetParams(char * buffer, UserData & data)
{
STG::RS::PACKET_TAIL packetTail;

DecryptString(&packetTail, buffer, sizeof(packetTail), &ctxS);

if (strncmp(reinterpret_cast<const char*>(packetTail.magic), RS_ID, RS_MAGIC_LEN))
    {
    printfd(__FILE__, "Invalid crypto magic\n");
    return true;
    }

std::ostringstream params;
params << "\"" << data.login << "\" "
       << inet_ntostring(data.ip) << " "
       << data.id << " "
       << reinterpret_cast<const char*>(packetTail.params);

data.params = params.str();

return false;
}
//-----------------------------------------------------------------------------
void LISTENER::ProcessPending()
{
auto it = pending.begin();
size_t count = 0;
printfd(__FILE__, "Pending: %d\n", pending.size());
while (it != pending.end() && count < 256)
    {
    auto uit = std::lower_bound(users.begin(), users.end(), it->data.login);
    if (it->type == PendingData::CONNECT)
        {
        printfd(__FILE__, "Connect packet\n");
        if (uit == users.end() || uit->data.login != it->data.login)
            {
            printfd(__FILE__, "Connect new user '%s'\n", it->data.login.c_str());
            // Add new user
            Connect(*it);
            users.insert(uit, AliveData(it->data));
            }
        else
            {
            printfd(__FILE__, "Update existing user '%s'\n", it->data.login.c_str());
            // Update already existing user
            time(&uit->lastAlive);
            uit->data.params = it->data.params;
            }
        }
    else if (it->type == PendingData::ALIVE)
        {
        printfd(__FILE__, "Alive packet\n");
        if (uit != users.end() && uit->data.login == it->data.login)
            {
            printfd(__FILE__, "Alive user '%s'\n", it->data.login.c_str());
            // Update existing user
            time(&uit->lastAlive);
            }
        else
            {
            printfd(__FILE__, "Alive user '%s' is not found\n", it->data.login.c_str());
            }
        }
    else if (it->type == PendingData::DISCONNECT)
        {
        printfd(__FILE__, "Disconnect packet\n");
        if (uit != users.end() && uit->data.login == it->data.login.c_str())
            {
            printfd(__FILE__, "Disconnect user '%s'\n", it->data.login.c_str());
            // Disconnect existing user
            uit->data.params = it->data.params;
            Disconnect(*uit);
            users.erase(uit);
            }
        else
            {
            printfd(__FILE__, "Cannot find user '%s' for disconnect\n", it->data.login.c_str());
            }
        }
    else
        {
        printfd(__FILE__, "Unknown packet type\n");
        }
    ++it;
    ++count;
    }
std::lock_guard lock(m_mutex);
pending.erase(pending.begin(), it);
}
//-----------------------------------------------------------------------------
void LISTENER::ProcessTimeouts()
{
const auto now = time(nullptr);
const auto it = std::stable_partition(users.begin(), users.end(), [this, now](const auto& data){ return difftime(now, data.lastAlive) < userTimeout; });

if (it != users.end())
    {
    printfd(__FILE__, "Total users: %d, users to disconnect: %d\n", users.size(), std::distance(it, users.end()));

    std::for_each(it, users.end(), [this](const auto& user){ Disconnect(user);});

    users.erase(it, users.end());
    }
}
//-----------------------------------------------------------------------------
bool LISTENER::Connect(const PendingData & pd) const
{
printfd(__FILE__, "Connect %s\n", pd.data.login.c_str());
if (access(scriptOnConnect.c_str(), X_OK) == 0)
    {
    if (ScriptExec((scriptOnConnect + " " + pd.data.params).c_str()))
        {
        WriteServLog("Script %s cannot be executed for an unknown reason.", scriptOnConnect.c_str());
        return true;
        }
    }
else
    {
    WriteServLog("Script %s cannot be executed. File not found.", scriptOnConnect.c_str());
    return true;
    }
return false;
}
//-----------------------------------------------------------------------------
bool LISTENER::Disconnect(const AliveData& ad) const
{
printfd(__FILE__, "Disconnect %s\n", ad.data.login.c_str());
if (access(scriptOnDisconnect.c_str(), X_OK) == 0)
    {
    if (ScriptExec((scriptOnDisconnect + " " + ad.data.params).c_str()))
        {
        WriteServLog("Script %s cannot be executed for an unknown reson.", scriptOnDisconnect.c_str());
        return true;
        }
    }
else
    {
    WriteServLog("Script %s cannot be executed. File not found.", scriptOnDisconnect.c_str());
    return true;
    }
return false;
}
//-----------------------------------------------------------------------------
bool LISTENER::CheckHeader(const STG::RS::PACKET_HEADER & header) const
{
if (strncmp(reinterpret_cast<const char*>(header.magic), RS_ID, RS_MAGIC_LEN))
    return true;
if (strncmp(reinterpret_cast<const char*>(header.protoVer), "02", RS_PROTO_VER_LEN))
    return true;
return false;
}
//-----------------------------------------------------------------------------
void InitEncrypt(BLOWFISH_CTX * ctx, const std::string & password)
{
char keyL[PASSWD_LEN];
memset(keyL, 0, PASSWD_LEN);
strncpy(keyL, password.c_str(), PASSWD_LEN);
Blowfish_Init(ctx, keyL, PASSWD_LEN);
}
//-----------------------------------------------------------------------------
