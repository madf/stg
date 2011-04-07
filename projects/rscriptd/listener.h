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

#include <pthread.h>

#include <string>
#include <vector>
#include <list>
#include <functional>

#include "stg/os_int.h"
#include "stg/blowfish.h"
#include "stg/rs_packets.h"
#include "stg/stg_logger.h"

struct UserData
{
    std::string params;
    std::string login;
    uint32_t    ip;
    uint32_t    id;
};

struct PendingData : public UserData
{
    enum {CONNECT, ALIVE, DISCONNECT} type;
};

struct AliveData : public UserData
{
    explicit AliveData(const UserData & data)
        : UserData(data),
          lastAlive(time(NULL))
    {};
    bool operator<(const std::string & rvalue) const { return login < rvalue; };
    time_t      lastAlive;
};

class IsNotTimedOut : public std::unary_function<const AliveData &, bool> {
    public:
        IsNotTimedOut(double to) : timeout(to), now(time(NULL)) {}
        bool operator()(const AliveData & data) const
        {
            return difftime(now, data.lastAlive) < timeout;
        }
    private:
        double timeout;
        time_t now;
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
    static void *       Run(void * self);
    static void *       RunProcessor(void * self);
    void                Runner();
    void                ProcessorRunner();
    // Networking stuff
    bool                PrepareNet();
    bool                FinalizeNet();
    bool                WaitPackets(int sd) const;
    bool                RecvPacket();
    // Parsing stuff
    bool                CheckHeader(const RS_PACKET_HEADER & header) const;
    bool                GetParams(char * buffer, UserData & data);
    // Processing stuff
    void                ProcessPending();
    void                ProcessTimeouts();
    bool                Disconnect(const UserData & data) const;
    bool                Connect(const UserData & data) const;

    BLOWFISH_CTX        ctxS;
    STG_LOGGER &        WriteServLog;

    mutable std::string errorStr;
    std::string         scriptOnConnect;
    std::string         scriptOnDisconnect;
    std::string         password;
    uint16_t            port;

    bool                running;
    bool                receiverStopped;
    bool                processorStopped;
    std::vector<AliveData> users;
    std::list<PendingData> pending;
    int                 userTimeout;

    pthread_t           receiverThread;
    pthread_t           processorThread;
    pthread_mutex_t     mutex;

    int                 listenSocket;

    std::string         version;

    friend class DisconnectUser;
};

class DisconnectUser : public std::unary_function<const UserData &, void> {
    public:
        DisconnectUser(LISTENER & l) : listener(l) {};
        void operator()(const UserData & data)
        {
            listener.Disconnect(data);
        };
    private:
        LISTENER & listener;
};
//-----------------------------------------------------------------------------
