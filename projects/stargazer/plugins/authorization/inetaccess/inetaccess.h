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
#pragma once

#include "stg/auth.h"
#include "stg/store.h"
#include "stg/module_settings.h"
#include "stg/notifer.h"
#include "stg/user_ips.h"
#include "stg/user.h"
#include "stg/users.h"
#include "stg/user_property.h"
#include "stg/ia_packets.h"
#include "stg/blowfish.h"
#include "stg/logger.h"
#include "stg/utime.h"
#include "stg/logger.h"

#include <cstring>
#include <ctime>
#include <cstdint>
#include <string>
#include <map>
#include <list>
#include <functional>
#include <utility>
#include <mutex>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop

#include <sys/time.h>

//#define IA_DEBUG (1)
//#define IA_PHASE_DEBUG (1)

class AUTH_IA;
//-----------------------------------------------------------------------------
enum FREEMB {
    freeMb0 = 0,
    freeMb1,
    freeMb2,
    freeMb3,
    freeMb4,
    freeMb5,
    freeMb6,
    freeMb7,
    freeMb8,
    freeMb9,
    freeMb10,
    freeMb11,
    freeMb12,
    freeMb13,
    freeMb14,
    freeMb15,
    freeMb16,
    freeMb17,
    freeMb18,
    freeMb19,
    freeMbCash = 100,
    freeMbNone = 101
};
//-----------------------------------------------------------------------------
class IA_PHASE {
public:
    IA_PHASE();
    ~IA_PHASE();

    void    SetPhase1();
    void    SetPhase2();
    void    SetPhase3();
    void    SetPhase4();
    int     GetPhase() const;

    void    UpdateTime();
    const UTIME & GetTime() const;

    #ifdef IA_PHASE_DEBUG
    void    SetUserLogin(const std::string & login);
    void    SetLogFileName(const std::string & logFileName);
    #endif

private:
    int             phase;
    UTIME           phaseTime;

    #ifdef IA_PHASE_DEBUG
    void WritePhaseChange(int newPhase);
    std::string log;
    std::string login;
    FILE * flog;
    #endif
};
//-----------------------------------------------------------------------------
struct IA_USER {
    using ConstUserPtr = const STG::User*;
    IA_USER()
        : user(NULL),
          lastSendAlive(0),
          rnd(static_cast<uint32_t>(random())),
          port(0),
          protoVer(0),
          password("NO PASSWORD")
    {
    char keyL[PASSWD_LEN];
    memset(keyL, 0, PASSWD_LEN);
    strncpy(keyL, password.c_str(), PASSWD_LEN);
    Blowfish_Init(&ctx, keyL, PASSWD_LEN);

    #ifdef IA_DEBUG
    aliveSent = false;
    #endif
    }

    IA_USER(const IA_USER & u)
        : login(u.login),
          user(u.user),
          phase(u.phase),
          lastSendAlive(u.lastSendAlive),
          rnd(u.rnd),
          port(u.port),
          ctx(),
          messagesToSend(u.messagesToSend),
          protoVer(u.protoVer),
          password(u.password)
    {
    #ifdef IA_DEBUG
    aliveSent  = u.aliveSent;
    #endif
    memcpy(&ctx, &u.ctx, sizeof(BLOWFISH_CTX));
    }

    IA_USER(const std::string & l,
            ConstUserPtr u,
            uint16_t p,
            int ver)
        : login(l),
          user(u),
          lastSendAlive(0),
          rnd(static_cast<uint32_t>(random())),
          port(p),
          messagesToSend(),
          protoVer(ver),
          password(user->GetProperties().password.Get())
    {
    char keyL[PASSWD_LEN];
    memset(keyL, 0, PASSWD_LEN);
    strncpy(keyL, password.c_str(), PASSWD_LEN);
    Blowfish_Init(&ctx, keyL, PASSWD_LEN);

    #ifdef IA_DEBUG
    aliveSent = false;
    #endif
    }

    std::string     login;
    ConstUserPtr  user;
    IA_PHASE        phase;
    UTIME           lastSendAlive;
    uint32_t        rnd;
    uint16_t        port;
    BLOWFISH_CTX    ctx;
    std::vector<STG::Message> messagesToSend;
    int             protoVer;
    std::string     password;
    #ifdef IA_DEBUG
    bool            aliveSent;
    #endif

private:
    IA_USER & operator=(const IA_USER & rvalue);
};
//-----------------------------------------------------------------------------
class AUTH_IA_SETTINGS {
public:
                    AUTH_IA_SETTINGS();
    virtual         ~AUTH_IA_SETTINGS() {}
    const std::string & GetStrError() const { return errorStr; }
    int             ParseSettings(const STG::ModuleSettings & s);
    UTIME           GetUserDelay() const { return UTIME(userDelay); }
    UTIME           GetUserTimeout() const { return UTIME(userTimeout); }
    uint16_t        GetUserPort() const { return port; }
    FREEMB          GetFreeMbShowType() const { return freeMbShowType; }
    bool            LogProtocolErrors() const { return logProtocolErrors; }

private:
    int             userDelay;
    int             userTimeout;
    uint16_t        port;
    std::string     errorStr;
    FREEMB          freeMbShowType;
    bool            logProtocolErrors;
};
//-----------------------------------------------------------------------------
class AUTH_IA;
using UserPtr = STG::User*;
//-----------------------------------------------------------------------------
class AUTH_IA : public STG::Auth {
public:
                        AUTH_IA();
                        ~AUTH_IA() override;

    void                SetUsers(STG::Users * u) override { users = u; }
    void                SetStgSettings(const STG::Settings * s) override { stgSettings = s; }
    void                SetSettings(const STG::ModuleSettings & s) override { settings = s; }
    int                 ParseSettings() override;

    int                 Start() override;
    int                 Stop() override;
    int                 Reload(const STG::ModuleSettings & ms) override;
    bool                IsRunning() override { return isRunningRunTimeouter || isRunningRun; }

    const std::string & GetStrError() const override { return errorStr; }
    std::string         GetVersion() const override { return "InetAccess authorization plugin v.1.4"; }
    uint16_t            GetStartPosition() const override { return 30; }
    uint16_t            GetStopPosition() const override { return 30; }

    int                 SendMessage(const STG::Message & msg, uint32_t ip) const override;

private:
    AUTH_IA(const AUTH_IA & rvalue);
    AUTH_IA & operator=(const AUTH_IA & rvalue);

    void                Run(std::stop_token token);
    void                RunTimeouter(std::stop_token token);
    int                 PrepareNet();
    int                 FinalizeNet();
    void                DelUser(UserPtr u);
    int                 RecvData(char * buffer, int bufferSize);
    int                 CheckHeader(const char * buffer, uint32_t sip, int * protoVer);
    int                 PacketProcessor(void * buff, size_t dataLen, uint32_t sip, uint16_t sport, int protoVer, UserPtr user);

    int                 Process_CONN_SYN_6(CONN_SYN_6 * connSyn, IA_USER * iaUser, uint32_t sip);
    int                 Process_CONN_SYN_7(CONN_SYN_7 * connSyn, IA_USER * iaUser, uint32_t sip);
    int                 Process_CONN_SYN_8(CONN_SYN_8 * connSyn, IA_USER * iaUser, uint32_t sip);

    int                 Process_CONN_ACK_6(CONN_ACK_6 * connAck, IA_USER * iaUser, uint32_t sip);
    int                 Process_CONN_ACK_7(CONN_ACK_7 * connAck, IA_USER * iaUser, uint32_t sip);
    int                 Process_CONN_ACK_8(CONN_ACK_8 * connAck, IA_USER * iaUser, uint32_t sip);

    int                 Process_ALIVE_ACK_6(ALIVE_ACK_6 * aliveAck, IA_USER * iaUser, uint32_t sip);
    int                 Process_ALIVE_ACK_7(ALIVE_ACK_7 * aliveAck, IA_USER * iaUser, uint32_t sip);
    int                 Process_ALIVE_ACK_8(ALIVE_ACK_8 * aliveAck, IA_USER * iaUser, uint32_t sip);

    int                 Process_DISCONN_SYN_6(DISCONN_SYN_6 * disconnSyn, IA_USER * iaUser, uint32_t sip);
    int                 Process_DISCONN_SYN_7(DISCONN_SYN_7 * disconnSyn, IA_USER * iaUser, uint32_t sip);
    int                 Process_DISCONN_SYN_8(DISCONN_SYN_8 * disconnSyn, IA_USER * iaUser, uint32_t sip);

    int                 Process_DISCONN_ACK_6(DISCONN_ACK_6 * disconnSyn,
                                              IA_USER * iaUser,
                                              uint32_t sip,
                                              std::map<uint32_t, IA_USER>::iterator it);
    int                 Process_DISCONN_ACK_7(DISCONN_ACK_7 * disconnSyn,
                                              IA_USER * iaUser,
                                              uint32_t sip,
                                              std::map<uint32_t, IA_USER>::iterator it);
    int                 Process_DISCONN_ACK_8(DISCONN_ACK_8 * disconnSyn,
                                              IA_USER * iaUser,
                                              uint32_t sip,
                                              std::map<uint32_t, IA_USER>::iterator it);

    int                 Send_CONN_SYN_ACK_6(IA_USER * iaUser, uint32_t sip);
    int                 Send_CONN_SYN_ACK_7(IA_USER * iaUser, uint32_t sip);
    int                 Send_CONN_SYN_ACK_8(IA_USER * iaUser, uint32_t sip);

    int                 Send_ALIVE_SYN_6(IA_USER * iaUser, uint32_t sip);
    int                 Send_ALIVE_SYN_7(IA_USER * iaUser, uint32_t sip);
    int                 Send_ALIVE_SYN_8(IA_USER * iaUser, uint32_t sip);

    int                 Send_DISCONN_SYN_ACK_6(IA_USER * iaUser, uint32_t sip);
    int                 Send_DISCONN_SYN_ACK_7(IA_USER * iaUser, uint32_t sip);
    int                 Send_DISCONN_SYN_ACK_8(IA_USER * iaUser, uint32_t sip);

    int                 Send_FIN_6(IA_USER * iaUser, uint32_t sip, std::map<uint32_t, IA_USER>::iterator it);
    int                 Send_FIN_7(IA_USER * iaUser, uint32_t sip, std::map<uint32_t, IA_USER>::iterator it);
    int                 Send_FIN_8(IA_USER * iaUser, uint32_t sip, std::map<uint32_t, IA_USER>::iterator it);

    int                 Timeouter();

    int                 SendError(uint32_t ip, uint16_t port, int protoVer, const std::string & text);
    int                 Send(uint32_t ip, uint16_t port, const void* buffer, size_t len);
    int                 RealSendMessage6(const STG::Message & msg, uint32_t ip, IA_USER & user);
    int                 RealSendMessage7(const STG::Message & msg, uint32_t ip, IA_USER & user);
    int                 RealSendMessage8(const STG::Message & msg, uint32_t ip, IA_USER & user);

    BLOWFISH_CTX        ctxS;        //for loginS

    mutable std::string errorStr;
    AUTH_IA_SETTINGS    iaSettings;
    STG::ModuleSettings settings;

    bool                isRunningRun;
    bool                isRunningRunTimeouter;

    STG::Users *             users;
    const STG::Settings *    stgSettings;

    mutable std::map<uint32_t, IA_USER> ip2user;

    std::jthread        m_thread;
    std::jthread        m_timeouterThread;
    mutable std::mutex  m_mutex;

    int                 listenSocket;

    CONN_SYN_ACK_6      connSynAck6;
    CONN_SYN_ACK_8      connSynAck8;

    DISCONN_SYN_ACK_6   disconnSynAck6;
    DISCONN_SYN_ACK_8   disconnSynAck8;

    ALIVE_SYN_6         aliveSyn6;
    ALIVE_SYN_8         aliveSyn8;
    FIN_6               fin6;
    FIN_8               fin8;

    std::map<std::string, int> packetTypes;

    uint32_t            enabledDirs;

    STG::ScopedConnection m_onDelUserConn;

    STG::PluginLogger   logger;

    friend class UnauthorizeUser;
};
//-----------------------------------------------------------------------------
class UnauthorizeUser : std::unary_function<const std::pair<uint32_t, IA_USER> &, void> {
    public:
        explicit UnauthorizeUser(AUTH_IA * a) : auth(a) {}
        UnauthorizeUser(const UnauthorizeUser & rvalue) : auth(rvalue.auth) {}
        void operator()(const std::pair<uint32_t, IA_USER> & p)
        {
            auth->users->Unauthorize(p.second.user->GetLogin(), auth);
        }
    private:
        UnauthorizeUser & operator=(const UnauthorizeUser & rvalue);

        AUTH_IA * auth;
};
