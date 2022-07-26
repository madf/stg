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

#include "stg/blowfish.h"
#include "stg/rs_packets.h"
#include "stg/logger.h"

#include <string>
#include <vector>
#include <list>
#include <functional>
#include <mutex>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop
#include <cstdint>

struct UserData
{
    std::string params;
    std::string login;
    uint32_t    ip;
    uint32_t    id;
};

struct PendingData
{
    UserData data;
    enum {CONNECT, ALIVE, DISCONNECT} type;
};

struct AliveData
{
    explicit AliveData(const UserData& ud)
        : data(ud),
          lastAlive(time(NULL))
    {};
    bool operator<(const std::string& rhs) const { return data.login < rhs; };
    UserData data;
    time_t lastAlive;
};

class LISTENER
{
public:
                        LISTENER();
                        ~LISTENER(){};

    void                SetPort(uint16_t p) { port = p; };
    void                SetPassword(const std::string & p);
    void                SetUserTimeout(int t) { userTimeout = t; };
    void                SetScriptOnConnect(const std::string & script) { scriptOnConnect = script; };
    void                SetScriptOnDisconnect(const std::string & script) { scriptOnDisconnect = script; };

    bool                Start();
    bool                Stop();
    bool                IsRunning() const { return !receiverStopped && !processorStopped; };

    const std::string & GetStrError() const { return errorStr; };
    const std::string & GetVersion() const { return version; };

private:
    // Threading stuff
    void                Run(std::stop_token token);
    void                RunProcessor(std::stop_token token);
    // Networking stuff
    bool                PrepareNet();
    bool                FinalizeNet();
    bool                RecvPacket(const std::stop_token& token);
    // Parsing stuff
    bool                CheckHeader(const RS::PACKET_HEADER & header) const;
    bool                GetParams(char * buffer, UserData & data);
    // Processing stuff
    void                ProcessPending();
    void                ProcessTimeouts();
    bool                Disconnect(const AliveData& data) const;
    bool                Connect(const PendingData& data) const;

    BLOWFISH_CTX        ctxS;
    STG::Logger&        WriteServLog;

    mutable std::string errorStr;
    std::string         scriptOnConnect;
    std::string         scriptOnDisconnect;
    std::string         password;
    uint16_t            port;

    bool                receiverStopped;
    bool                processorStopped;
    std::vector<AliveData> users;
    std::vector<PendingData> pending;
    int                 userTimeout;

    std::jthread        m_receiverThread;
    std::jthread        m_processorThread;
    std::mutex          m_mutex;

    int                 listenSocket;

    std::string         version;
};
