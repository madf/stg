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

/*
 $Revision: 1.34 $
 $Date: 2010/09/10 06:39:19 $
 $Author: faust $
 */

#ifndef INETACCESS_H
#define INETACCESS_H

#include <sys/time.h>
#include <pthread.h>
#include <cstring>
#include <ctime>
#include <string>
#include <map>
#include <functional>
#include <utility>

#include "os_int.h"
#include "base_auth.h"
#include "base_store.h"
#include "notifer.h"
#include "user_ips.h"
#include "../../../user.h"
#include "../../../users.h"
#include "ia_packets.h"
#include "blowfish.h"
#include "stg_logger.h"
#include "utime.h"

using namespace std;

extern "C" BASE_PLUGIN * GetPlugin();

#define IA_PROTO_VER    (6)

//#define IA_DEBUG (1)
//#define IA_PHASE_DEBUG (1)

class AUTH_IA;
//-----------------------------------------------------------------------------
enum FREEMB
{
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
class IA_PHASE
{
public:
    IA_PHASE();
    ~IA_PHASE();

    void    SetPhase1();
    void    SetPhase2();
    void    SetPhase3();
    void    SetPhase4();
    void    SetPhase5();
    int     GetPhase() const;

    void    UpdateTime();
    const UTIME & GetTime() const;

    #ifdef IA_PHASE_DEBUG
    void    SetUserLogin(const string & login);
    void    SetLogFileName(const string & logFileName);
    #endif

private:
    int             phase;
    UTIME           phaseTime;

    #ifdef IA_PHASE_DEBUG
    void WritePhaseChange(int newPhase);
    string log;
    string login;
    FILE * flog;
    #endif
};
//-----------------------------------------------------------------------------
struct IA_USER
{
    IA_USER()
        : lastSendAlive(0),
          rnd(random()),
          port(0),
          protoVer(0),
          password("NO PASSWORD")
    {
        // +++ Preparing CTX +++
        unsigned char keyL[PASSWD_LEN];  // Пароль для шифровки
        memset(keyL, 0, PASSWD_LEN);
        strncpy((char *)keyL, password.c_str(), PASSWD_LEN);
        Blowfish_Init(&ctx, keyL, PASSWD_LEN);
        // --- Preparing CTX ---
        #ifdef IA_DEBUG
        aliveSent = false;
        #endif
    };

    IA_USER(const IA_USER & u)
        : user(u.user),
          phase(u.phase),
          lastSendAlive(u.lastSendAlive),
          rnd(u.rnd),
          port(u.port),
          messagesToSend(u.messagesToSend),
          protoVer(u.protoVer),
          password(u.password)
    {
        #ifdef IA_DEBUG
        aliveSent  = u.aliveSent;
        #endif
        memcpy(&ctx, &u.ctx, sizeof(BLOWFISH_CTX));
    };

    user_iter       user;
    //int             phase;
    //UTIME           phaseTime;
    IA_PHASE        phase;
    UTIME           lastSendAlive;
    uint32_t        rnd;
    uint16_t        port;
    BLOWFISH_CTX    ctx;
    list<STG_MSG>   messagesToSend;
    int             protoVer;
    string          password;
    #ifdef IA_DEBUG
    bool            aliveSent;
    #endif
};
//-----------------------------------------------------------------------------
class AUTH_IA_SETTINGS
{
public:
                    AUTH_IA_SETTINGS();
    virtual         ~AUTH_IA_SETTINGS() {};
    const string&   GetStrError() const { return errorStr; };
    int             ParseSettings(const MODULE_SETTINGS & s);
    int             GetUserDelay() const { return userDelay; };
    int             GetUserTimeout() const { return userTimeout; };
    int             GetUserPort() const { return port; };
    FREEMB          GetFreeMbShowType() const { return freeMbShowType; };

private:
    int             ParseIntInRange(const string & str, int min, int max, int * val);
    int             userDelay;
    int             userTimeout;
    uint16_t        port;
    string          errorStr;
    FREEMB          freeMbShowType;
};
//-----------------------------------------------------------------------------
class AUTH_IA :public BASE_AUTH
{
public:
                        AUTH_IA();
    virtual             ~AUTH_IA();

    void                SetUsers(USERS * u) { users = u; };
    void                SetTariffs(TARIFFS *){};
    void                SetAdmins(ADMINS *){};
    void                SetTraffcounter(TRAFFCOUNTER *){};
    void                SetStore(BASE_STORE *){};
    void                SetStgSettings(const SETTINGS * s) { stgSettings = s; };
    void                SetSettings(const MODULE_SETTINGS & s) { settings = s; };
    int                 ParseSettings();

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; };
    bool                IsRunning() { return isRunningRunTimeouter || isRunningRun; };

    const string      & GetStrError() const { return errorStr; };
    const string        GetVersion() const { return "InetAccess authorization plugin v.1.4"; };
    uint16_t            GetStartPosition() const { return 50; };
    uint16_t            GetStopPosition() const { return 50; };

    void                DelUser(user_iter u);

    int                 SendMessage(const STG_MSG & msg, uint32_t ip) const;

private:
    static void *       Run(void *);
    static void *       RunTimeouter(void * d);
    int                 PrepareNet();
    int                 FinalizeNet();
    int                 RecvData(char * buffer, int bufferSize);
    int                 CheckHeader(const char * buffer, int * protoVer);
    int                 PacketProcessor(char * buff, int dataLen, uint32_t sip, uint16_t sport, int protoVer, user_iter * user);

    int                 Process_CONN_SYN_6(CONN_SYN_6 * connSyn, IA_USER * iaUser, user_iter * user, uint32_t sip);
    int                 Process_CONN_SYN_7(CONN_SYN_7 * connSyn, IA_USER * iaUser, user_iter * user, uint32_t sip);
    int                 Process_CONN_SYN_8(CONN_SYN_8 * connSyn, IA_USER * iaUser, user_iter * user, uint32_t sip);

    int                 Process_CONN_ACK_6(CONN_ACK_6 * connAck, IA_USER * iaUser, user_iter * user, uint32_t sip);
    int                 Process_CONN_ACK_7(CONN_ACK_7 * connAck, IA_USER * iaUser, user_iter * user, uint32_t sip);
    int                 Process_CONN_ACK_8(CONN_ACK_8 * connAck, IA_USER * iaUser, user_iter * user, uint32_t sip);

    int                 Process_ALIVE_ACK_6(ALIVE_ACK_6 * aliveAck, IA_USER * iaUser, user_iter * user, uint32_t sip);
    int                 Process_ALIVE_ACK_7(ALIVE_ACK_7 * aliveAck, IA_USER * iaUser, user_iter * user, uint32_t sip);
    int                 Process_ALIVE_ACK_8(ALIVE_ACK_8 * aliveAck, IA_USER * iaUser, user_iter * user, uint32_t sip);

    int                 Process_DISCONN_SYN_6(DISCONN_SYN_6 * disconnSyn, IA_USER * iaUser, user_iter * user, uint32_t sip);
    int                 Process_DISCONN_SYN_7(DISCONN_SYN_7 * disconnSyn, IA_USER * iaUser, user_iter * user, uint32_t sip);
    int                 Process_DISCONN_SYN_8(DISCONN_SYN_8 * disconnSyn, IA_USER * iaUser, user_iter * user, uint32_t sip);

    int                 Process_DISCONN_ACK_6(DISCONN_ACK_6 * disconnSyn,
                                              IA_USER * iaUser,
                                              user_iter * user,
                                              uint32_t sip,
                                              map<uint32_t, IA_USER>::iterator it);
    int                 Process_DISCONN_ACK_7(DISCONN_ACK_7 * disconnSyn,
                                              IA_USER * iaUser,
                                              user_iter * user,
                                              uint32_t sip,
                                              map<uint32_t, IA_USER>::iterator it);
    int                 Process_DISCONN_ACK_8(DISCONN_ACK_8 * disconnSyn,
                                              IA_USER * iaUser,
                                              user_iter * user,
                                              uint32_t sip,
                                              map<uint32_t, IA_USER>::iterator it);

    int                 Send_CONN_SYN_ACK_6(IA_USER * iaUser, user_iter * user, uint32_t sip);
    int                 Send_CONN_SYN_ACK_7(IA_USER * iaUser, user_iter * user, uint32_t sip);
    int                 Send_CONN_SYN_ACK_8(IA_USER * iaUser, user_iter * user, uint32_t sip);

    int                 Send_ALIVE_SYN_6(IA_USER * iaUser, uint32_t sip);
    int                 Send_ALIVE_SYN_7(IA_USER * iaUser, uint32_t sip);
    int                 Send_ALIVE_SYN_8(IA_USER * iaUser, uint32_t sip);

    int                 Send_DISCONN_SYN_ACK_6(IA_USER * iaUser, uint32_t sip);
    int                 Send_DISCONN_SYN_ACK_7(IA_USER * iaUser, uint32_t sip);
    int                 Send_DISCONN_SYN_ACK_8(IA_USER * iaUser, uint32_t sip);

    int                 Send_FIN_6(IA_USER * iaUser, uint32_t sip, map<uint32_t, IA_USER>::iterator it);
    int                 Send_FIN_7(IA_USER * iaUser, uint32_t sip, map<uint32_t, IA_USER>::iterator it);
    int                 Send_FIN_8(IA_USER * iaUser, uint32_t sip, map<uint32_t, IA_USER>::iterator it);

    int                 Timeouter();

    int                 SendError(uint32_t ip, uint16_t port, int protoVer, const string & text);
    int                 Send(uint32_t ip, uint16_t port, const char * buffer, int len);
    int                 RealSendMessage6(const STG_MSG & msg, uint32_t ip, IA_USER & user);
    int                 RealSendMessage7(const STG_MSG & msg, uint32_t ip, IA_USER & user);
    int                 RealSendMessage8(const STG_MSG & msg, uint32_t ip, IA_USER & user);

    bool                WaitPackets(int sd) const;

    BLOWFISH_CTX        ctxS;        //for loginS

    mutable string      errorStr;
    AUTH_IA_SETTINGS    iaSettings;
    MODULE_SETTINGS     settings;

    bool                nonstop;

    bool                isRunningRun;
    bool                isRunningRunTimeouter;

    USERS *             users;
    const SETTINGS *    stgSettings;

    mutable map<uint32_t, IA_USER>  ip2user;

    pthread_t           recvThread;
    pthread_t           timeouterThread;
    mutable pthread_mutex_t mutex;

    int                 listenSocket;

    CONN_SYN_ACK_6      connSynAck6;
    CONN_SYN_ACK_8      connSynAck8;

    DISCONN_SYN_ACK_6   disconnSynAck6;
    DISCONN_SYN_ACK_8   disconnSynAck8;

    ALIVE_SYN_6         aliveSyn6;
    ALIVE_SYN_8         aliveSyn8;
    FIN_6               fin6;
    FIN_8               fin8;

    map<string, int>    packetTypes;

    STG_LOGGER &        WriteServLog;

    uint32_t            enabledDirs;

    class DEL_USER_NONIFIER: public NOTIFIER_BASE<user_iter>
    {
    public:
        DEL_USER_NONIFIER(AUTH_IA & a) : auth(a) {};
        virtual ~DEL_USER_NONIFIER(){};

        void Notify(const user_iter & user)
            {
            auth.DelUser(user);
            }

    private:
        AUTH_IA & auth;
    } onDelUserNotifier;

    class UnauthorizeUser : std::unary_function<const std::pair<uint32_t, IA_USER> &, void> {
        public:
            UnauthorizeUser(AUTH_IA * a) : auth(a) {};
            void operator()(const std::pair<uint32_t, IA_USER> & p)
            {
                p.second.user->Unauthorize(auth);
            }
        private:
            AUTH_IA * auth;
    };

};
//-----------------------------------------------------------------------------

#endif


