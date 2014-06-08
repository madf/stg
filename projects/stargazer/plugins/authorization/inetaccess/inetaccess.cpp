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
 $Revision: 1.79 $
 $Date: 2010/03/25 15:18:48 $
 $Author: faust $
 */
 
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h> // close

#include <csignal>
#include <cstdlib>
#include <cstdio> // snprintf
#include <cerrno>
#include <cmath>
#include <algorithm>

#include "stg/common.h"
#include "stg/locker.h"
#include "stg/tariff.h"
#include "stg/user_property.h"
#include "stg/settings.h"
#include "stg/plugin_creator.h"
#include "inetaccess.h"

extern volatile time_t stgTime;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
namespace
{
PLUGIN_CREATOR<AUTH_IA> iac;

void InitEncrypt(BLOWFISH_CTX * ctx, const std::string & password);
void Decrypt(BLOWFISH_CTX * ctx, void * dst, const void * src, size_t len8);
void Encrypt(BLOWFISH_CTX * ctx, void * dst, const void * src, size_t len8);
}

extern "C" PLUGIN * GetPlugin();
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN * GetPlugin()
{
return iac.GetPlugin();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
AUTH_IA_SETTINGS::AUTH_IA_SETTINGS()
    : userDelay(0),
      userTimeout(0),
      port(0),
      errorStr(),
      freeMbShowType(freeMbCash),
      logProtocolErrors(false)
{
}
//-----------------------------------------------------------------------------
int AUTH_IA_SETTINGS::ParseSettings(const MODULE_SETTINGS & s)
{
int p;
PARAM_VALUE pv;
std::vector<PARAM_VALUE>::const_iterator pvi;
///////////////////////////
pv.param = "Port";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
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
port = static_cast<uint16_t>(p);
///////////////////////////
pv.param = "UserDelay";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end())
    {
    errorStr = "Parameter \'UserDelay\' not found.";
    printfd(__FILE__, "Parameter 'UserDelay' not found\n");
    return -1;
    }

if (ParseIntInRange(pvi->value[0], 5, 600, &userDelay))
    {
    errorStr = "Cannot parse parameter \'UserDelay\': " + errorStr;
    printfd(__FILE__, "Cannot parse parameter 'UserDelay'\n");
    return -1;
    }
///////////////////////////
pv.param = "UserTimeout";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end())
    {
    errorStr = "Parameter \'UserTimeout\' not found.";
    printfd(__FILE__, "Parameter 'UserTimeout' not found\n");
    return -1;
    }

if (ParseIntInRange(pvi->value[0], 15, 1200, &userTimeout))
    {
    errorStr = "Cannot parse parameter \'UserTimeout\': " + errorStr;
    printfd(__FILE__, "Cannot parse parameter 'UserTimeout'\n");
    return -1;
    }
///////////////////////////
pv.param = "LogProtocolErrors";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end())
    logProtocolErrors = false;
else if (ParseYesNo(pvi->value[0], &logProtocolErrors))
    {
    errorStr = "Cannot parse parameter \'LogProtocolErrors\': " + errorStr;
    printfd(__FILE__, "Cannot parse parameter 'LogProtocolErrors'\n");
    return -1;
    }
/////////////////////////////////////////////////////////////
std::string freeMbType;
int n = 0;
pv.param = "FreeMb";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end())
    {
    errorStr = "Parameter \'FreeMb\' not found.";
    printfd(__FILE__, "Parameter 'FreeMb' not found\n");
    return -1;
    }
freeMbType = pvi->value[0];

if (strcasecmp(freeMbType.c_str(), "cash") == 0)
    {
    freeMbShowType = freeMbCash;
    }
else if (strcasecmp(freeMbType.c_str(), "none") == 0)
    {
    freeMbShowType = freeMbNone;
    }
else if (!str2x(freeMbType.c_str(), n))
    {
    if (n < 0 || n >= DIR_NUM)
        {
        errorStr = "Incorrect parameter \'" + freeMbType + "\'.";
        printfd(__FILE__, "%s\n", errorStr.c_str());
        return -1;
        }
    freeMbShowType = (FREEMB)(freeMb0 + n);
    }
else
    {
    errorStr = "Incorrect parameter \'" + freeMbType + "\'.";
    printfd(__FILE__, "%s\n", errorStr.c_str());
    return -1;
    }
/////////////////////////////////////////////////////////////
return 0;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifdef IA_PHASE_DEBUG
IA_PHASE::IA_PHASE()
    : phase(1),
      phaseTime(),
      flog(NULL)
{
gettimeofday(&phaseTime, NULL);
}
#else
IA_PHASE::IA_PHASE()
    : phase(1),
      phaseTime()
{
gettimeofday(&phaseTime, NULL);
}
#endif
//-----------------------------------------------------------------------------
IA_PHASE::~IA_PHASE()
{
#ifdef IA_PHASE_DEBUG
flog = fopen(log.c_str(), "at");
if (flog)
    {
    fprintf(flog, "IA %s D\n", login.c_str());
    fclose(flog);
    }
#endif
}
//-----------------------------------------------------------------------------
#ifdef IA_PHASE_DEBUG
void IA_PHASE::SetLogFileName(const string & logFileName)
{
log = logFileName + ".ia.log";
}
//-----------------------------------------------------------------------------
void IA_PHASE::SetUserLogin(const string & login)
{
IA_PHASE::login = login;
}
//-----------------------------------------------------------------------------
void IA_PHASE::WritePhaseChange(int newPhase)
{
UTIME newPhaseTime;
gettimeofday(&newPhaseTime, NULL);
flog = fopen(log.c_str(), "at");
if (flog)
    {
    string action = newPhase == phase ? "U" : "C";
    double delta = newPhaseTime.GetSec() - phaseTime.GetSec();
    delta += (newPhaseTime.GetUSec() - phaseTime.GetUSec()) * 1.0e-6;
    fprintf(flog, "IA %s %s oldPhase = %d, newPhase = %d. dt = %.6f\n",
            login.c_str(),
            action.c_str(),
            phase,
            newPhase,
            delta);
    fclose(flog);
    }
}
#endif
//-----------------------------------------------------------------------------
void IA_PHASE::SetPhase1()
{
#ifdef IA_PHASE_DEBUG
WritePhaseChange(1);
#endif
phase = 1;
gettimeofday(&phaseTime, NULL);
}
//-----------------------------------------------------------------------------
void IA_PHASE::SetPhase2()
{
#ifdef IA_PHASE_DEBUG
WritePhaseChange(2);
#endif
phase = 2;
gettimeofday(&phaseTime, NULL);
}
//-----------------------------------------------------------------------------
void IA_PHASE::SetPhase3()
{
#ifdef IA_PHASE_DEBUG
WritePhaseChange(3);
#endif
phase = 3;
gettimeofday(&phaseTime, NULL);
}
//-----------------------------------------------------------------------------
void IA_PHASE::SetPhase4()
{
#ifdef IA_PHASE_DEBUG
WritePhaseChange(4);
#endif
phase = 4;
gettimeofday(&phaseTime, NULL);
}
//-----------------------------------------------------------------------------
int IA_PHASE::GetPhase() const
{
return phase;
}
//-----------------------------------------------------------------------------
void IA_PHASE::UpdateTime()
{
#ifdef IA_PHASE_DEBUG
WritePhaseChange(phase);
#endif
gettimeofday(&phaseTime, NULL);
}
//-----------------------------------------------------------------------------
const UTIME & IA_PHASE::GetTime() const
{
return phaseTime;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
AUTH_IA::AUTH_IA()
    : ctxS(),
      errorStr(),
      iaSettings(),
      settings(),
      nonstop(false),
      isRunningRun(false),
      isRunningRunTimeouter(false),
      users(NULL),
      stgSettings(NULL),
      ip2user(),
      recvThread(),
      timeouterThread(),
      mutex(),
      listenSocket(-1),
      connSynAck6(),
      connSynAck8(),
      disconnSynAck6(),
      disconnSynAck8(),
      aliveSyn6(),
      aliveSyn8(),
      fin6(),
      fin8(),
      packetTypes(),
      enabledDirs(0xFFffFFff),
      onDelUserNotifier(*this),
      logger(GetPluginLogger(GetStgLogger(), "auth_ia"))
{
InitEncrypt(&ctxS, "pr7Hhen");

pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
pthread_mutex_init(&mutex, &attr);

memset(&connSynAck6, 0, sizeof(CONN_SYN_ACK_6));
memset(&connSynAck8, 0, sizeof(CONN_SYN_ACK_8));
memset(&disconnSynAck6, 0, sizeof(DISCONN_SYN_ACK_6));
memset(&disconnSynAck8, 0, sizeof(DISCONN_SYN_ACK_8));
memset(&aliveSyn6, 0, sizeof(ALIVE_SYN_6));
memset(&aliveSyn8, 0, sizeof(ALIVE_SYN_8));
memset(&fin6, 0, sizeof(FIN_6));
memset(&fin8, 0, sizeof(FIN_8));

printfd(__FILE__, "sizeof(CONN_SYN_6) = %d %d\n",           sizeof(CONN_SYN_6),     Min8(sizeof(CONN_SYN_6)));
printfd(__FILE__, "sizeof(CONN_SYN_8) = %d %d\n",           sizeof(CONN_SYN_8),     Min8(sizeof(CONN_SYN_8)));
printfd(__FILE__, "sizeof(CONN_SYN_ACK_6) = %d %d\n",       sizeof(CONN_SYN_ACK_6), Min8(sizeof(CONN_SYN_ACK_6)));
printfd(__FILE__, "sizeof(CONN_SYN_ACK_8) = %d %d\n",       sizeof(CONN_SYN_ACK_8), Min8(sizeof(CONN_SYN_ACK_8)));
printfd(__FILE__, "sizeof(CONN_ACK_6) = %d %d\n",           sizeof(CONN_ACK_6),     Min8(sizeof(CONN_ACK_6)));
printfd(__FILE__, "sizeof(ALIVE_SYN_6) = %d %d\n",          sizeof(ALIVE_SYN_6),    Min8(sizeof(ALIVE_SYN_6)));
printfd(__FILE__, "sizeof(ALIVE_SYN_8) = %d %d\n",          sizeof(ALIVE_SYN_8),    Min8(sizeof(ALIVE_SYN_8)));
printfd(__FILE__, "sizeof(ALIVE_ACK_6) = %d %d\n",          sizeof(ALIVE_ACK_6),    Min8(sizeof(ALIVE_ACK_6)));
printfd(__FILE__, "sizeof(DISCONN_SYN_6) = %d %d\n",        sizeof(DISCONN_SYN_6),  Min8(sizeof(DISCONN_SYN_6)));
printfd(__FILE__, "sizeof(DISCONN_SYN_ACK_6) = %d %d\n",    sizeof(DISCONN_SYN_ACK_6), Min8(sizeof(DISCONN_SYN_ACK_6)));
printfd(__FILE__, "sizeof(DISCONN_SYN_ACK_8) = %d %d\n",    sizeof(DISCONN_SYN_ACK_8), Min8(sizeof(DISCONN_SYN_ACK_8)));
printfd(__FILE__, "sizeof(DISCONN_ACK_6) = %d %d\n",        sizeof(DISCONN_ACK_6),  Min8(sizeof(DISCONN_ACK_6)));
printfd(__FILE__, "sizeof(FIN_6) = %d %d\n",                sizeof(FIN_6),          Min8(sizeof(FIN_6)));
printfd(__FILE__, "sizeof(FIN_8) = %d %d\n",                sizeof(FIN_8),          Min8(sizeof(FIN_8)));
printfd(__FILE__, "sizeof(ERR) = %d %d\n",                  sizeof(ERR),            Min8(sizeof(ERR)));
printfd(__FILE__, "sizeof(INFO_6) = %d %d\n",               sizeof(INFO_6),         Min8(sizeof(INFO_6)));
printfd(__FILE__, "sizeof(INFO_7) = %d %d\n",               sizeof(INFO_7),         Min8(sizeof(INFO_7)));
printfd(__FILE__, "sizeof(INFO_8) = %d %d\n",               sizeof(INFO_8),         Min8(sizeof(INFO_8)));

packetTypes["CONN_SYN"] = CONN_SYN_N;
packetTypes["CONN_SYN_ACK"] = CONN_SYN_ACK_N;
packetTypes["CONN_ACK"] = CONN_ACK_N;
packetTypes["ALIVE_SYN"] = ALIVE_SYN_N;
packetTypes["ALIVE_ACK"] = ALIVE_ACK_N;
packetTypes["DISCONN_SYN"] = DISCONN_SYN_N;
packetTypes["DISCONN_SYN_ACK"] = DISCONN_SYN_ACK_N;
packetTypes["DISCONN_ACK"] = DISCONN_ACK_N;
packetTypes["FIN"] = FIN_N;
packetTypes["ERR"] = ERROR_N;
}
//-----------------------------------------------------------------------------
AUTH_IA::~AUTH_IA()
{
pthread_mutex_destroy(&mutex);
}
//-----------------------------------------------------------------------------
int AUTH_IA::Start()
{
users->AddNotifierUserDel(&onDelUserNotifier);
nonstop = true;

if (PrepareNet())
    {
    return -1;
    }

if (!isRunningRun)
    {
    if (pthread_create(&recvThread, NULL, Run, this))
        {
        errorStr = "Cannot create thread.";
        printfd(__FILE__, "Cannot create recv thread\n");
        logger("Cannot create recv thread.");
        return -1;
        }
    }

if (!isRunningRunTimeouter)
    {
    if (pthread_create(&timeouterThread, NULL, RunTimeouter, this))
        {
        errorStr = "Cannot create thread.";
        printfd(__FILE__, "Cannot create timeouter thread\n");
        logger("Cannot create timeouter thread.");
        return -1;
        }
    }
errorStr = "";
return 0;
}
//-----------------------------------------------------------------------------
int AUTH_IA::Stop()
{
if (!IsRunning())
    return 0;

nonstop = false;

std::for_each(
        ip2user.begin(),
        ip2user.end(),
        UnauthorizeUser(this)
        );

if (isRunningRun)
    {
    //5 seconds to thread stops itself
    for (int i = 0; i < 25 && isRunningRun; i++)
        {
        struct timespec ts = {0, 200000000};
        nanosleep(&ts, NULL);
        }
    }

FinalizeNet();

if (isRunningRunTimeouter)
    {
    //5 seconds to thread stops itself
    for (int i = 0; i < 25 && isRunningRunTimeouter; i++)
        {
        struct timespec ts = {0, 200000000};
        nanosleep(&ts, NULL);
        }
    }

users->DelNotifierUserDel(&onDelUserNotifier);

if (isRunningRun || isRunningRunTimeouter)
    return -1;

printfd(__FILE__, "AUTH_IA::Stoped successfully.\n");
return 0;
}
//-----------------------------------------------------------------------------
void * AUTH_IA::Run(void * d)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

AUTH_IA * ia = static_cast<AUTH_IA *>(d);

ia->isRunningRun = true;

char buffer[512];

time_t touchTime = stgTime - MONITOR_TIME_DELAY_SEC;

while (ia->nonstop)
    {
    ia->RecvData(buffer, sizeof(buffer));
    if ((touchTime + MONITOR_TIME_DELAY_SEC <= stgTime) && ia->stgSettings->GetMonitoring())
        {
        touchTime = stgTime;
        std::string monFile = ia->stgSettings->GetMonitorDir() + "/inetaccess_r";
        TouchFile(monFile.c_str());
        }
    }

ia->isRunningRun = false;
return NULL;
}
//-----------------------------------------------------------------------------
void * AUTH_IA::RunTimeouter(void * d)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

AUTH_IA * ia = static_cast<AUTH_IA *>(d);

ia->isRunningRunTimeouter = true;

int a = -1;
std::string monFile = ia->stgSettings->GetMonitorDir() + "/inetaccess_t";
while (ia->nonstop)
    {
    struct timespec ts = {0, 20000000};
    nanosleep(&ts, NULL);
    ia->Timeouter();
    // TODO change counter to timer and MONITOR_TIME_DELAY_SEC
    if (++a % (50 * 60) == 0 && ia->stgSettings->GetMonitoring())
        {
        TouchFile(monFile.c_str());
        }
    }

ia->isRunningRunTimeouter = false;
return NULL;
}
//-----------------------------------------------------------------------------
int AUTH_IA::ParseSettings()
{
int ret = iaSettings.ParseSettings(settings);
if (ret)
    errorStr = iaSettings.GetStrError();
return ret;
}
//-----------------------------------------------------------------------------
int AUTH_IA::PrepareNet()
{
struct sockaddr_in listenAddr;

listenSocket = socket(AF_INET, SOCK_DGRAM, 0);

if (listenSocket < 0)
    {
    errorStr = "Cannot create socket.";
    logger("Cannot create a socket: %s", strerror(errno));
    return -1;
    }

listenAddr.sin_family = AF_INET;
listenAddr.sin_port = htons(static_cast<uint16_t>(iaSettings.GetUserPort()));
listenAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

if (bind(listenSocket, (struct sockaddr*)&listenAddr, sizeof(listenAddr)) < 0)
    {
    errorStr = "AUTH_IA: Bind failed.";
    logger("Cannot bind the socket: %s", strerror(errno));
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int AUTH_IA::FinalizeNet()
{
close(listenSocket);
return 0;
}
//-----------------------------------------------------------------------------
int AUTH_IA::RecvData(char * buffer, int bufferSize)
{
if (!WaitPackets(listenSocket)) // Timeout
    {
    return 0;
    }

struct sockaddr_in outerAddr;
socklen_t outerAddrLen(sizeof(outerAddr));
ssize_t dataLen = recvfrom(listenSocket, buffer, bufferSize, 0, (struct sockaddr *)&outerAddr, &outerAddrLen);

if (!dataLen) // EOF
    {
    return 0;
    }

if (dataLen <= 0) // Error
    {
    if (errno != EINTR)
        {
        printfd(__FILE__, "recvfrom res=%d, error: '%s'\n", dataLen, strerror(errno));
        logger("recvfrom error: %s", strerror(errno));
        return -1;
        }
    return 0;
    }

if (dataLen > 256)
    return -1;

uint32_t sip = outerAddr.sin_addr.s_addr;
uint16_t sport = htons(outerAddr.sin_port);

int protoVer;
if (CheckHeader(buffer, sip, &protoVer))
    return -1;

char login[PASSWD_LEN];  //TODO why PASSWD_LEN ?
memset(login, 0, PASSWD_LEN);

Decrypt(&ctxS, login, buffer + 8, PASSWD_LEN / 8);

USER_PTR user;
if (users->FindByName(login, &user))
    {
    logger("User's connect failed: user '%s' not found. IP %s",
           login,
           inet_ntostring(sip).c_str());
    printfd(__FILE__, "User '%s' NOT found!\n", login);
    SendError(sip, sport, protoVer, "Неправильный логин!");
    return -1;
    }

printfd(__FILE__, "User '%s' FOUND!\n", user->GetLogin().c_str());

if (user->GetProperty().disabled.Get())
    {
    logger("Cannont authorize '%s', user is disabled.", login);
    SendError(sip, sport, protoVer, "Учетная запись заблокирована");
    return 0;
    }

if (user->GetProperty().passive.Get())
    {
    logger("Cannont authorize '%s', user is passive.", login);
    SendError(sip, sport, protoVer, "Учетная запись заморожена");
    return 0;
    }

if (!user->GetProperty().ips.Get().IsIPInIPS(sip))
    {
    printfd(__FILE__, "User %s. IP address is incorrect. IP %s\n",
            user->GetLogin().c_str(), inet_ntostring(sip).c_str());
    logger("User %s. IP address is incorrect. IP %s",
           user->GetLogin().c_str(), inet_ntostring(sip).c_str());
    SendError(sip, sport, protoVer, "Пользователь не опознан! Проверьте IP адрес.");
    return 0;
    }

return PacketProcessor(buffer, dataLen, sip, sport, protoVer, user);
}
//-----------------------------------------------------------------------------
int AUTH_IA::CheckHeader(const char * buffer, uint32_t sip, int * protoVer)
{
if (strncmp(IA_ID, buffer, strlen(IA_ID)) != 0)
    {
    //SendError(userIP, updateMsg);
    printfd(__FILE__, "update needed - IA_ID\n");
    if (iaSettings.LogProtocolErrors())
        logger("IP: %s. Header: invalid packed signature.", inet_ntostring(sip).c_str());
    //SendError(userIP, "Incorrect header!");
    return -1;
    }

if (buffer[6] != 0) //proto[0] shoud be 0
    {
    printfd(__FILE__, "update needed - PROTO major: %d\n", buffer[6]);
    if (iaSettings.LogProtocolErrors())
        logger("IP: %s. Header: invalid protocol major version: %d.", inet_ntostring(sip).c_str(), buffer[6]);
    //SendError(userIP, updateMsg);
    return -1;
    }

if (buffer[7] < 6)
    {
    // need update
    //SendError(userIP, updateMsg);
    printfd(__FILE__, "update needed - PROTO minor: %d\n", buffer[7]);
    if (iaSettings.LogProtocolErrors())
        logger("IP: %s. Header: invalid protocol minor version: %d.", inet_ntostring(sip).c_str(), buffer[7]);
    return -1;
    }
else
    {
    *protoVer = buffer[7];
    }
return 0;
}
//-----------------------------------------------------------------------------
int AUTH_IA::Timeouter()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

std::map<uint32_t, IA_USER>::iterator it;
it = ip2user.begin();

while (it != ip2user.end())
    {
    uint32_t sip = it->first;

    static UTIME currTime;
    gettimeofday(&currTime, NULL);

    if ((it->second.phase.GetPhase() == 2)
        && (currTime - it->second.phase.GetTime()) > iaSettings.GetUserDelay())
        {
        if (iaSettings.LogProtocolErrors())
            logger("User '%s'. Protocol version: %d. Phase 2: connect request timeout (%f > %d).", it->second.login.c_str(), it->second.protoVer, (currTime - it->second.phase.GetTime()).AsDouble(), iaSettings.GetUserDelay());
        it->second.phase.SetPhase1();
        printfd(__FILE__, "Phase changed from 2 to 1. Reason: timeout\n");
        ip2user.erase(it++);
        continue;
        }

    if (it->second.phase.GetPhase() == 3)
        {
        if (!it->second.messagesToSend.empty())
            {
            if (it->second.protoVer == 6)
                RealSendMessage6(*it->second.messagesToSend.begin(), sip, it->second);

            if (it->second.protoVer == 7)
                RealSendMessage7(*it->second.messagesToSend.begin(), sip, it->second);

            if (it->second.protoVer == 8)
                RealSendMessage8(*it->second.messagesToSend.begin(), sip, it->second);

            it->second.messagesToSend.erase(it->second.messagesToSend.begin());
            }

        if((currTime - it->second.lastSendAlive) > iaSettings.GetUserDelay())
            {
            switch (it->second.protoVer)
                {
                case 6:
                    Send_ALIVE_SYN_6(&(it->second), sip);
                    break;
                case 7:
                    Send_ALIVE_SYN_7(&(it->second), sip);
                    break;
                case 8:
                    Send_ALIVE_SYN_8(&(it->second), sip);
                    break;
                }

            gettimeofday(&it->second.lastSendAlive, NULL);
            }

        if ((currTime - it->second.phase.GetTime()) > iaSettings.GetUserTimeout())
            {
            if (iaSettings.LogProtocolErrors())
                logger("User '%s'. Protocol version: %d. Phase 3: alive timeout (%f > %d).", it->second.login.c_str(), it->second.protoVer, (currTime - it->second.phase.GetTime()).AsDouble(), iaSettings.GetUserTimeout());
            users->Unauthorize(it->second.user->GetLogin(), this);
            ip2user.erase(it++);
            continue;
            }
        }

    if ((it->second.phase.GetPhase() == 4)
        && ((currTime - it->second.phase.GetTime()) > iaSettings.GetUserDelay()))
        {
        if (iaSettings.LogProtocolErrors())
            logger("User '%s'. Protocol version: %d. Phase 4: disconnect request timeout (%f > %d).", it->second.login.c_str(), it->second.protoVer, (currTime - it->second.phase.GetTime()).AsDouble(), iaSettings.GetUserDelay());
        it->second.phase.SetPhase3();
        printfd(__FILE__, "Phase changed from 4 to 3. Reason: timeout\n");
        }

    ++it;
    }

return 0;
}
//-----------------------------------------------------------------------------
int AUTH_IA::PacketProcessor(void * buff, size_t dataLen, uint32_t sip, uint16_t sport, int protoVer, USER_PTR user)
{
std::string login(user->GetLogin());
const size_t offset = LOGIN_LEN + 2 + 6; // LOGIN_LEN + sizeOfMagic + sizeOfVer;

STG_LOCKER lock(&mutex, __FILE__, __LINE__);
std::map<uint32_t, IA_USER>::iterator it(ip2user.find(sip));

if (it == ip2user.end())
    {
    USER_PTR userPtr;
    if (!users->FindByIPIdx(sip, &userPtr))
        {
        if (userPtr->GetID() != user->GetID())
            {
            printfd(__FILE__, "IP address already in use by user '%s'. IP %s, login: '%s'\n",
                    userPtr->GetLogin().c_str(),
                    inet_ntostring(sip).c_str(),
                   login.c_str());
            logger("IP address is already in use by user '%s'. IP %s, login: '%s'",
                   userPtr->GetLogin().c_str(),
                   inet_ntostring(sip).c_str(),
                   login.c_str());
            SendError(sip, sport, protoVer, "Ваш IP адрес уже используется!");
            return 0;
            }
        }

    printfd(__FILE__, "Add new user '%s' from ip %s\n",
            login.c_str(), inet_ntostring(sip).c_str());
    std::pair<std::map<uint32_t, IA_USER>::iterator, bool> res;
    res = ip2user.insert(std::make_pair(sip, IA_USER(login, user, sport, protoVer)));
    it = res.first;
    #ifdef IA_PHASE_DEBUG
    it->second.phase.SetLogFileName(stgSettings->GetLogFileName());
    it->second.phase.SetUserLogin(login);
    #endif
    }
else if (user->GetID() != it->second.user->GetID())
    {
    printfd(__FILE__, "IP address already in use by user '%s'. IP %s, login: '%s'\n",
            it->second.user->GetLogin().c_str(),
            inet_ntostring(sip).c_str(),
            user->GetLogin().c_str());
    logger("IP address is already in use by user '%s'. IP %s, login: '%s'",
           it->second.user->GetLogin().c_str(),
           inet_ntostring(sip).c_str(),
           user->GetLogin().c_str());
    SendError(sip, sport, protoVer, "Ваш IP адрес уже используется!");
    return 0;
    }

IA_USER * iaUser = &(it->second);

if (iaUser->password != user->GetProperty().password.Get())
    {
    InitEncrypt(&iaUser->ctx, user->GetProperty().password.Get());
    iaUser->password = user->GetProperty().password.Get();
    }

Decrypt(&iaUser->ctx, static_cast<char *>(buff) + offset, static_cast<char *>(buff) + offset, (dataLen - offset) / 8);

char packetName[IA_MAX_TYPE_LEN];
strncpy(packetName,  static_cast<char *>(buff) + offset + 4, IA_MAX_TYPE_LEN);
packetName[IA_MAX_TYPE_LEN - 1] = 0;

std::map<std::string, int>::iterator pi(packetTypes.find(packetName));
if (pi == packetTypes.end())
    {
    SendError(sip, sport, protoVer, "Неправильный логин или пароль!");
    printfd(__FILE__, "Login or password is wrong!\n");
    logger("User's connect failed. User: '%s', ip %s. Wrong login or password",
           login.c_str(),
           inet_ntostring(sip).c_str());
    ip2user.erase(it);
    return 0;
    }

if (user->IsAuthorizedBy(this) && user->GetCurrIP() != sip)
    {
    printfd(__FILE__, "Login %s already in use from ip %s. IP %s\n",
            login.c_str(), inet_ntostring(user->GetCurrIP()).c_str(),
            inet_ntostring(sip).c_str());
    logger("Login '%s' is already in use from ip %s. IP %s",
           login.c_str(),
           inet_ntostring(user->GetCurrIP()).c_str(),
           inet_ntostring(sip).c_str());
    SendError(sip, sport, protoVer, "Ваш логин уже используется!");
    ip2user.erase(it);
    return 0;
    }

switch (pi->second)
    {
    case CONN_SYN_N:
        switch (protoVer)
            {
            case 6:
                if (Process_CONN_SYN_6(static_cast<CONN_SYN_6 *>(buff), &(it->second), sip))
                    return -1;
                return Send_CONN_SYN_ACK_6(iaUser, sip);
            case 7:
                if (Process_CONN_SYN_7(static_cast<CONN_SYN_7 *>(buff), &(it->second), sip))
                    return -1;
                return Send_CONN_SYN_ACK_7(iaUser, sip);
            case 8:
                if (Process_CONN_SYN_8(static_cast<CONN_SYN_8 *>(buff), &(it->second), sip))
                    return -1;
                return Send_CONN_SYN_ACK_8(iaUser, sip);
            }
        break;

    case CONN_ACK_N:
        switch (protoVer)
            {
            case 6:
                if (Process_CONN_ACK_6(static_cast<CONN_ACK_6 *>(buff), iaUser, sip))
                    return -1;
                return Send_ALIVE_SYN_6(iaUser, sip);
            case 7:
                if (Process_CONN_ACK_7(static_cast<CONN_ACK_6 *>(buff), iaUser, sip))
                    return -1;
                return Send_ALIVE_SYN_7(iaUser, sip);
            case 8:
                if (Process_CONN_ACK_8(static_cast<CONN_ACK_8 *>(buff), iaUser, sip))
                    return -1;
                return Send_ALIVE_SYN_8(iaUser, sip);
            }
        break;

    case ALIVE_ACK_N:
        switch (protoVer)
            {
            case 6:
                return Process_ALIVE_ACK_6(static_cast<ALIVE_ACK_6 *>(buff), iaUser, sip);
            case 7:
                return Process_ALIVE_ACK_7(static_cast<ALIVE_ACK_6 *>(buff), iaUser, sip);
            case 8:
                return Process_ALIVE_ACK_8(static_cast<ALIVE_ACK_8 *>(buff), iaUser, sip);
            }
        break;

    case DISCONN_SYN_N:
        switch (protoVer)
            {
            case 6:
                if (Process_DISCONN_SYN_6(static_cast<DISCONN_SYN_6 *>(buff), iaUser, sip))
                    return -1;
                return Send_DISCONN_SYN_ACK_6(iaUser, sip);
            case 7:
                if (Process_DISCONN_SYN_7(static_cast<DISCONN_SYN_6 *>(buff), iaUser, sip))
                    return -1;
                return Send_DISCONN_SYN_ACK_7(iaUser, sip);
            case 8:
                if (Process_DISCONN_SYN_8(static_cast<DISCONN_SYN_8 *>(buff), iaUser, sip))
                    return -1;
                return Send_DISCONN_SYN_ACK_8(iaUser, sip);
            }
        break;

    case DISCONN_ACK_N:
        switch (protoVer)
            {
            case 6:
                if (Process_DISCONN_ACK_6(static_cast<DISCONN_ACK_6 *>(buff), iaUser, sip, it))
                    return -1;
                return Send_FIN_6(iaUser, sip, it);
            case 7:
                if (Process_DISCONN_ACK_7(static_cast<DISCONN_ACK_6 *>(buff), iaUser, sip, it))
                    return -1;
                return Send_FIN_7(iaUser, sip, it);
            case 8:
                if (Process_DISCONN_ACK_8(static_cast<DISCONN_ACK_8 *>(buff), iaUser, sip, it))
                    return -1;
                return Send_FIN_8(iaUser, sip, it);
            }
        break;
    }

return -1;
}
//-----------------------------------------------------------------------------
void AUTH_IA::DelUser(USER_PTR u)
{

uint32_t ip = u->GetCurrIP();

if (!ip)
    return;

std::map<uint32_t, IA_USER>::iterator it;

STG_LOCKER lock(&mutex, __FILE__, __LINE__);
it = ip2user.find(ip);
if (it == ip2user.end())
    {
    //Nothing to delete
    printfd(__FILE__, "Nothing to delete\n");
    return;
    }

if (it->second.user == u)
    {
    printfd(__FILE__, "User removed!\n");
    users->Unauthorize(u->GetLogin(), this);
    ip2user.erase(it);
    }
}
//-----------------------------------------------------------------------------
int AUTH_IA::SendError(uint32_t ip, uint16_t port, int protoVer, const std::string & text)
{
struct sockaddr_in sendAddr;
ssize_t res;
switch (protoVer)
    {
    case 6:
    case 7:
        ERR err;
        memset(&err, 0, sizeof(ERR));

        sendAddr.sin_family = AF_INET;
        sendAddr.sin_port = htons(port);
        sendAddr.sin_addr.s_addr = ip;

        err.len = 1;
        strncpy((char*)err.type, "ERR", 16);
        strncpy((char*)err.text, text.c_str(), MAX_MSG_LEN);

        #ifdef ARCH_BE
        SwapBytes(err.len);
        #endif

        res = sendto(listenSocket, &err, sizeof(err), 0, (struct sockaddr*)&sendAddr, sizeof(sendAddr));
        printfd(__FILE__, "SendError %d bytes sent\n", res);
        break;

    case 8:
        ERR_8 err8;
        memset(&err8, 0, sizeof(ERR_8));

        sendAddr.sin_family = AF_INET;
        sendAddr.sin_port = htons(port);
        sendAddr.sin_addr.s_addr = ip;

        err8.len = 256;
        strncpy((char*)err8.type, "ERR", 16);
        strncpy((char*)err8.text, text.c_str(), MAX_MSG_LEN);

        #ifdef ARCH_BE
        SwapBytes(err8.len);
        #endif

        res = sendto(listenSocket, &err8, sizeof(err8), 0, (struct sockaddr*)&sendAddr, sizeof(sendAddr));
        printfd(__FILE__, "SendError_8 %d bytes sent\n", res);
        break;
    }

return 0;
}
//-----------------------------------------------------------------------------
int AUTH_IA::Send(uint32_t ip, uint16_t port, const char * buffer, size_t len)
{
struct sockaddr_in sendAddr;

sendAddr.sin_family = AF_INET;
sendAddr.sin_port = htons(port);
sendAddr.sin_addr.s_addr = ip;

if (sendto(listenSocket, buffer, len, 0, (struct sockaddr*)&sendAddr, sizeof(sendAddr)) == static_cast<ssize_t>(len))
    return 0;

return -1;
}
//-----------------------------------------------------------------------------
int AUTH_IA::SendMessage(const STG_MSG & msg, uint32_t ip) const
{
printfd(__FILE__, "SendMessage userIP=%s\n", inet_ntostring(ip).c_str());

std::map<uint32_t, IA_USER>::iterator it;

STG_LOCKER lock(&mutex, __FILE__, __LINE__);
it = ip2user.find(ip);
if (it == ip2user.end())
    {
    errorStr = "Unknown user.";
    return -1;
    }
it->second.messagesToSend.push_back(msg);
return 0;
}
//-----------------------------------------------------------------------------
int AUTH_IA::RealSendMessage6(const STG_MSG & msg, uint32_t ip, IA_USER & user)
{
printfd(__FILE__, "RealSendMessage 6 user=%s\n", user.login.c_str());

INFO_6 info;
memset(&info, 0, sizeof(INFO_6));

info.len = 256;
strncpy((char*)info.type, "INFO", 16);
info.infoType = 'I';
strncpy((char*)info.text, msg.text.c_str(), 235);
info.text[234] = 0;

size_t len = info.len;
#ifdef ARCH_BE
SwapBytes(info.len);
#endif

char buffer[256];
memcpy(buffer, &info, sizeof(INFO_6));
Encrypt(&user.ctx, buffer, buffer, len / 8);
return Send(ip, iaSettings.GetUserPort(), buffer, len);
}
//-----------------------------------------------------------------------------
int AUTH_IA::RealSendMessage7(const STG_MSG & msg, uint32_t ip, IA_USER & user)
{
printfd(__FILE__, "RealSendMessage 7 user=%s\n", user.login.c_str());

INFO_7 info;
memset(&info, 0, sizeof(INFO_7));

info.len = 264;
strncpy((char*)info.type, "INFO_7", 16);
info.infoType = static_cast<int8_t>(msg.header.type);
info.showTime = static_cast<int8_t>(msg.header.showTime);
info.sendTime = msg.header.creationTime;

size_t len = info.len;
#ifdef ARCH_BE
SwapBytes(info.len);
SwapBytes(info.sendTime);
#endif

strncpy((char*)info.text, msg.text.c_str(), MAX_MSG_LEN - 1);
info.text[MAX_MSG_LEN - 1] = 0;

char buffer[300];
memcpy(buffer, &info, sizeof(INFO_7));

Encrypt(&user.ctx, buffer, buffer, len / 8);
return Send(ip, iaSettings.GetUserPort(), buffer, len);
}
//-----------------------------------------------------------------------------
int AUTH_IA::RealSendMessage8(const STG_MSG & msg, uint32_t ip, IA_USER & user)
{
printfd(__FILE__, "RealSendMessage 8 user=%s\n", user.login.c_str());

INFO_8 info;
memset(&info, 0, sizeof(INFO_8));

info.len = 1056;
strncpy((char*)info.type, "INFO_8", 16);
info.infoType = static_cast<int8_t>(msg.header.type);
info.showTime = static_cast<int8_t>(msg.header.showTime);
info.sendTime = msg.header.creationTime;

strncpy((char*)info.text, msg.text.c_str(), IA_MAX_MSG_LEN_8 - 1);
info.text[IA_MAX_MSG_LEN_8 - 1] = 0;

size_t len = info.len;
#ifdef ARCH_BE
SwapBytes(info.len);
SwapBytes(info.sendTime);
#endif

char buffer[1500];
memcpy(buffer, &info, sizeof(INFO_8));

Encrypt(&user.ctx, buffer, buffer, len / 8);
return Send(ip, user.port, buffer, len);
}
//-----------------------------------------------------------------------------
int AUTH_IA::Process_CONN_SYN_6(CONN_SYN_6 *, IA_USER * iaUser, uint32_t)
{
if (!(iaUser->phase.GetPhase() == 1 || iaUser->phase.GetPhase() == 3))
    return -1;

enabledDirs = 0xFFffFFff;

iaUser->phase.SetPhase2();
printfd(__FILE__, "Phase changed from %d to 2. Reason: CONN_SYN_6\n", iaUser->phase.GetPhase());
return 0;
}
//-----------------------------------------------------------------------------
int AUTH_IA::Process_CONN_SYN_7(CONN_SYN_7 * connSyn, IA_USER * iaUser, uint32_t sip)
{
return Process_CONN_SYN_6(connSyn, iaUser, sip);
}
//-----------------------------------------------------------------------------
int AUTH_IA::Process_CONN_SYN_8(CONN_SYN_8 * connSyn, IA_USER * iaUser, uint32_t sip)
{
#ifdef ARCH_BE
SwapBytes(connSyn->dirs);
#endif
int ret = Process_CONN_SYN_6(reinterpret_cast<CONN_SYN_6 *>(connSyn), iaUser, sip);
enabledDirs = connSyn->dirs;
return ret;
}
//-----------------------------------------------------------------------------
int AUTH_IA::Process_CONN_ACK_6(CONN_ACK_6 * connAck, IA_USER * iaUser, uint32_t sip)
{
#ifdef ARCH_BE
SwapBytes(connAck->len);
SwapBytes(connAck->rnd);
#endif
printfd( __FILE__, "CONN_ACK_6 %s\n", connAck->type);

if ((iaUser->phase.GetPhase() == 2) && (connAck->rnd == iaUser->rnd + 1))
    {
    iaUser->phase.UpdateTime();

    iaUser->lastSendAlive = iaUser->phase.GetTime();
    if (users->Authorize(iaUser->login, sip, enabledDirs, this))
        {
        iaUser->phase.SetPhase3();
        printfd(__FILE__, "Phase changed from 2 to 3. Reason: CONN_ACK_6\n");
        return 0;
        }
    else
        {
        errorStr = iaUser->user->GetStrError();
        if (iaSettings.LogProtocolErrors())
            logger("IP: %s. User '%s'. Protocol version: %d. CONN_ACK: phase 2, authorization error ('%s').", inet_ntostring(sip).c_str(), iaUser->login.c_str(), iaUser->protoVer, errorStr.c_str());
        iaUser->phase.SetPhase1();
        ip2user.erase(sip);
        printfd(__FILE__, "Phase changed from 2 to 1. Reason: failed to authorize user\n");
        return -1;
        }
    }
printfd(__FILE__, "Invalid phase or control number. Phase: %d. Control number: %d\n", iaUser->phase.GetPhase(), connAck->rnd);
if (iaSettings.LogProtocolErrors())
    {
    if (iaUser->phase.GetPhase() != 2)
        logger("IP: %s. User '%s'. Protocol version: %d. CONN_ACK: invalid phase, expected 2, got %d.", inet_ntostring(sip).c_str(), iaUser->login.c_str(), iaUser->protoVer, iaUser->phase.GetPhase());
    if (connAck->rnd != iaUser->rnd + 1)
        logger("IP: %s. User '%s'. Protocol version: %d. CONN_ACK: invalid control number, expected %d, got %d.", inet_ntostring(sip).c_str(), iaUser->login.c_str(), iaUser->protoVer, (iaUser->rnd + 1), connAck->rnd);
    }
return -1;
}
//-----------------------------------------------------------------------------
int AUTH_IA::Process_CONN_ACK_7(CONN_ACK_7 * connAck, IA_USER * iaUser, uint32_t sip)
{
return Process_CONN_ACK_6(connAck, iaUser, sip);
}
//-----------------------------------------------------------------------------
int AUTH_IA::Process_CONN_ACK_8(CONN_ACK_8 * connAck, IA_USER * iaUser, uint32_t sip)
{
#ifdef ARCH_BE
SwapBytes(connAck->len);
SwapBytes(connAck->rnd);
#endif
printfd( __FILE__, "CONN_ACK_8 %s\n", connAck->type);

if ((iaUser->phase.GetPhase() == 2) && (connAck->rnd == iaUser->rnd + 1))
    {
    iaUser->phase.UpdateTime();
    iaUser->lastSendAlive = iaUser->phase.GetTime();
    if (users->Authorize(iaUser->login, sip, enabledDirs, this))
        {
        iaUser->phase.SetPhase3();
        printfd(__FILE__, "Phase changed from 2 to 3. Reason: CONN_ACK_8\n");
        return 0;
        }
    else
        {
        errorStr = iaUser->user->GetStrError();
        if (iaSettings.LogProtocolErrors())
            logger("IP: %s. User '%s'. Protocol version: %d. CONN_ACK: phase 2, authorization error ('%s').", inet_ntostring(sip).c_str(), iaUser->login.c_str(), iaUser->protoVer, errorStr.c_str());
        iaUser->phase.SetPhase1();
        ip2user.erase(sip);
        printfd(__FILE__, "Phase changed from 2 to 1. Reason: failed to authorize user\n");
        return -1;
        }
    }
printfd(__FILE__, "Invalid phase or control number. Phase: %d. Control number: %d\n", iaUser->phase.GetPhase(), connAck->rnd);
if (iaSettings.LogProtocolErrors())
    {
    if (iaUser->phase.GetPhase() != 2)
        logger("IP: %s. User '%s'. Protocol version: %d. CONN_ACK: invalid phase, expected 2, got %d.", inet_ntostring(sip).c_str(), iaUser->login.c_str(), iaUser->protoVer, iaUser->phase.GetPhase());
    if (connAck->rnd != iaUser->rnd + 1)
        logger("IP: %s. User '%s'. Protocol version: %d. CONN_ACK: invalid control number, expected %d, got %d.", inet_ntostring(sip).c_str(), iaUser->login.c_str(), iaUser->protoVer, (iaUser->rnd + 1), connAck->rnd);
    }
return -1;
}
//-----------------------------------------------------------------------------
int AUTH_IA::Process_ALIVE_ACK_6(ALIVE_ACK_6 * aliveAck, IA_USER * iaUser, uint32_t)
{
#ifdef ARCH_BE
SwapBytes(aliveAck->len);
SwapBytes(aliveAck->rnd);
#endif
printfd(__FILE__, "ALIVE_ACK_6\n");
if ((iaUser->phase.GetPhase() == 3) && (aliveAck->rnd == iaUser->rnd + 1))
    {
    iaUser->phase.UpdateTime();
    #ifdef IA_DEBUG
    iaUser->aliveSent = false;
    #endif
    }
return 0;
}
//-----------------------------------------------------------------------------
int AUTH_IA::Process_ALIVE_ACK_7(ALIVE_ACK_7 * aliveAck, IA_USER * iaUser, uint32_t sip)
{
return Process_ALIVE_ACK_6(aliveAck, iaUser, sip);
}
//-----------------------------------------------------------------------------
int AUTH_IA::Process_ALIVE_ACK_8(ALIVE_ACK_8 * aliveAck, IA_USER * iaUser, uint32_t)
{
#ifdef ARCH_BE
SwapBytes(aliveAck->len);
SwapBytes(aliveAck->rnd);
#endif
printfd(__FILE__, "ALIVE_ACK_8\n");
if ((iaUser->phase.GetPhase() == 3) && (aliveAck->rnd == iaUser->rnd + 1))
    {
    iaUser->phase.UpdateTime();
    #ifdef IA_DEBUG
    iaUser->aliveSent = false;
    #endif
    }
return 0;
}
//-----------------------------------------------------------------------------
int AUTH_IA::Process_DISCONN_SYN_6(DISCONN_SYN_6 *, IA_USER * iaUser, uint32_t sip)
{
printfd(__FILE__, "DISCONN_SYN_6\n");
if (iaUser->phase.GetPhase() != 3)
    {
    if (iaSettings.LogProtocolErrors())
        logger("IP: %s. User '%s'. Protocol version: %d. DISCONN_SYN: invalid phase, expected 3, got %d.", inet_ntostring(sip).c_str(), iaUser->login.c_str(), iaUser->protoVer, iaUser->phase.GetPhase());
    printfd(__FILE__, "Invalid phase. Expected 3, actual %d\n", iaUser->phase.GetPhase());
    errorStr = "Incorrect request DISCONN_SYN";
    return -1;
    }

iaUser->phase.SetPhase4();
printfd(__FILE__, "Phase changed from 3 to 4. Reason: DISCONN_SYN_6\n");

return 0;
}
//-----------------------------------------------------------------------------
int AUTH_IA::Process_DISCONN_SYN_7(DISCONN_SYN_7 * disconnSyn, IA_USER * iaUser, uint32_t sip)
{
return Process_DISCONN_SYN_6(disconnSyn, iaUser, sip);
}
//-----------------------------------------------------------------------------
int AUTH_IA::Process_DISCONN_SYN_8(DISCONN_SYN_8 *, IA_USER * iaUser, uint32_t sip)
{
if (iaUser->phase.GetPhase() != 3)
    {
    if (iaSettings.LogProtocolErrors())
        logger("IP: %s. User '%s'. Protocol version: %d. DISCONN_SYN: invalid phase, expected 3, got %d.", inet_ntostring(sip).c_str(), iaUser->login.c_str(), iaUser->protoVer, iaUser->phase.GetPhase());
    errorStr = "Incorrect request DISCONN_SYN";
    printfd(__FILE__, "Invalid phase. Expected 3, actual %d\n", iaUser->phase.GetPhase());
    return -1;
    }

iaUser->phase.SetPhase4();
printfd(__FILE__, "Phase changed from 3 to 4. Reason: DISCONN_SYN_6\n");

return 0;
}
//-----------------------------------------------------------------------------
int AUTH_IA::Process_DISCONN_ACK_6(DISCONN_ACK_6 * disconnAck,
                                   IA_USER * iaUser,
                                   uint32_t sip,
                                   std::map<uint32_t, IA_USER>::iterator)
{
#ifdef ARCH_BE
SwapBytes(disconnAck->len);
SwapBytes(disconnAck->rnd);
#endif
printfd(__FILE__, "DISCONN_ACK_6\n");
if (!((iaUser->phase.GetPhase() == 4) && (disconnAck->rnd == iaUser->rnd + 1)))
    {
    if (iaSettings.LogProtocolErrors())
        {
        if (iaUser->phase.GetPhase() != 4)
            logger("IP: %s. User '%s'. Protocol version: %d. DISCONN_ACK: invalid phase, expected 4, got %d.", inet_ntostring(sip).c_str(), iaUser->login.c_str(), iaUser->protoVer, iaUser->phase.GetPhase());
        if (disconnAck->rnd != iaUser->rnd + 1)
            logger("IP: %s. User '%s'. Protocol version: %d. DISCONN_ACK: invalid control number, expected %d, got %d.", inet_ntostring(sip).c_str(), iaUser->login.c_str(), iaUser->protoVer, (iaUser->rnd + 1), disconnAck->rnd);
        }
    printfd(__FILE__, "Invalid phase or control number. Phase: %d. Control number: %d\n", iaUser->phase.GetPhase(), disconnAck->rnd);
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int AUTH_IA::Process_DISCONN_ACK_7(DISCONN_ACK_7 * disconnAck, IA_USER * iaUser, uint32_t sip, std::map<uint32_t, IA_USER>::iterator it)
{
return Process_DISCONN_ACK_6(disconnAck, iaUser, sip, it);
}
//-----------------------------------------------------------------------------
int AUTH_IA::Process_DISCONN_ACK_8(DISCONN_ACK_8 * disconnAck, IA_USER * iaUser, uint32_t sip, std::map<uint32_t, IA_USER>::iterator)
{
#ifdef ARCH_BE
SwapBytes(disconnAck->len);
SwapBytes(disconnAck->rnd);
#endif
printfd(__FILE__, "DISCONN_ACK_8\n");
if (!((iaUser->phase.GetPhase() == 4) && (disconnAck->rnd == iaUser->rnd + 1)))
    {
    if (iaSettings.LogProtocolErrors())
        {
        if (iaUser->phase.GetPhase() != 4)
            logger("IP: %s. User '%s'. Protocol version: %d. DISCONN_ACK: invalid phase, expected 4, got %d.", inet_ntostring(sip).c_str(), iaUser->login.c_str(), iaUser->protoVer, iaUser->phase.GetPhase());
        if (disconnAck->rnd != iaUser->rnd + 1)
            logger("IP: %s. User '%s'. Protocol version: %d. DISCONN_ACK: invalid control number, expected %d, got %d.", inet_ntostring(sip).c_str(), iaUser->login.c_str(), iaUser->protoVer, (iaUser->rnd + 1), disconnAck->rnd);
        }
    printfd(__FILE__, "Invalid phase or control number. Phase: %d. Control number: %d\n", iaUser->phase.GetPhase(), disconnAck->rnd);
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int AUTH_IA::Send_CONN_SYN_ACK_6(IA_USER * iaUser, uint32_t sip)
{
//+++ Fill static data in connSynAck +++
// TODO Move this code. It must be executed only once
connSynAck6.len = Min8(sizeof(CONN_SYN_ACK_6));
strcpy((char*)connSynAck6.type, "CONN_SYN_ACK");
for (int j = 0; j < DIR_NUM; j++)
    {
    strncpy((char*)connSynAck6.dirName[j],
            stgSettings->GetDirName(j).c_str(),
            sizeof(string16));

    connSynAck6.dirName[j][sizeof(string16) - 1] = 0;
    }
//--- Fill static data in connSynAck ---

iaUser->rnd = static_cast<uint32_t>(random());
connSynAck6.rnd = iaUser->rnd;

connSynAck6.userTimeOut = iaSettings.GetUserTimeout();
connSynAck6.aliveDelay = iaSettings.GetUserDelay();

#ifdef ARCH_BE
SwapBytes(connSynAck6.len);
SwapBytes(connSynAck6.rnd);
SwapBytes(connSynAck6.userTimeOut);
SwapBytes(connSynAck6.aliveDelay);
#endif

Encrypt(&iaUser->ctx, (char*)&connSynAck6, (char*)&connSynAck6, Min8(sizeof(CONN_SYN_ACK_6))/8);
return Send(sip, iaSettings.GetUserPort(), (char*)&connSynAck6, Min8(sizeof(CONN_SYN_ACK_6)));;
}
//-----------------------------------------------------------------------------
int AUTH_IA::Send_CONN_SYN_ACK_7(IA_USER * iaUser, uint32_t sip)
{
return Send_CONN_SYN_ACK_6(iaUser, sip);
}
//-----------------------------------------------------------------------------
int AUTH_IA::Send_CONN_SYN_ACK_8(IA_USER * iaUser, uint32_t sip)
{
strcpy((char*)connSynAck8.hdr.magic, IA_ID);
connSynAck8.hdr.protoVer[0] = 0;
connSynAck8.hdr.protoVer[1] = 8;

//+++ Fill static data in connSynAck +++
// TODO Move this code. It must be executed only once
connSynAck8.len = Min8(sizeof(CONN_SYN_ACK_8));
strcpy((char*)connSynAck8.type, "CONN_SYN_ACK");
for (int j = 0; j < DIR_NUM; j++)
    {
    strncpy((char*)connSynAck8.dirName[j],
            stgSettings->GetDirName(j).c_str(),
            sizeof(string16));

    connSynAck8.dirName[j][sizeof(string16) - 1] = 0;
    }
//--- Fill static data in connSynAck ---

iaUser->rnd = static_cast<uint32_t>(random());
connSynAck8.rnd = iaUser->rnd;

connSynAck8.userTimeOut = iaSettings.GetUserTimeout();
connSynAck8.aliveDelay = iaSettings.GetUserDelay();

#ifdef ARCH_BE
SwapBytes(connSynAck8.len);
SwapBytes(connSynAck8.rnd);
SwapBytes(connSynAck8.userTimeOut);
SwapBytes(connSynAck8.aliveDelay);
#endif

Encrypt(&iaUser->ctx, (char*)&connSynAck8, (char*)&connSynAck8, Min8(sizeof(CONN_SYN_ACK_8))/8);
return Send(sip, iaUser->port, (char*)&connSynAck8, Min8(sizeof(CONN_SYN_ACK_8)));
}
//-----------------------------------------------------------------------------
int AUTH_IA::Send_ALIVE_SYN_6(IA_USER * iaUser, uint32_t sip)
{
aliveSyn6.len = Min8(sizeof(ALIVE_SYN_6));
aliveSyn6.rnd = iaUser->rnd = static_cast<uint32_t>(random());

strcpy((char*)aliveSyn6.type, "ALIVE_SYN");

for (int i = 0; i < DIR_NUM; i++)
    {
    aliveSyn6.md[i] = iaUser->user->GetProperty().down.Get()[i];
    aliveSyn6.mu[i] = iaUser->user->GetProperty().up.Get()[i];

    aliveSyn6.sd[i] = iaUser->user->GetSessionDownload()[i];
    aliveSyn6.su[i] = iaUser->user->GetSessionUpload()[i];
    }

//TODO
int dn = iaSettings.GetFreeMbShowType();
const TARIFF * tf = iaUser->user->GetTariff();

if (dn < DIR_NUM)
    {
    double p = tf->GetPriceWithTraffType(aliveSyn6.mu[dn],
                                         aliveSyn6.md[dn],
                                         dn,
                                         stgTime);
    p *= 1024 * 1024;
    if (std::fabs(p) < 1.0e-3)
        {
        snprintf((char*)aliveSyn6.freeMb, IA_FREEMB_LEN, "---");
        }
    else
        {
        double fmb = iaUser->user->GetProperty().freeMb;
        fmb = fmb < 0 ? 0 : fmb;
        snprintf((char*)aliveSyn6.freeMb, IA_FREEMB_LEN, "%.3f", fmb / p);
        }
    }
else
    {
    if (freeMbNone == iaSettings.GetFreeMbShowType())
        {
        aliveSyn6.freeMb[0] = 0;
        }
    else
        {
        double fmb = iaUser->user->GetProperty().freeMb;
        fmb = fmb < 0 ? 0 : fmb;
        snprintf((char*)aliveSyn6.freeMb, IA_FREEMB_LEN, "C%.3f", fmb);
        }
    }

#ifdef IA_DEBUG
if (iaUser->aliveSent)
    {
    printfd(__FILE__, "========= ALIVE_ACK_6(7) TIMEOUT !!! %s =========\n", iaUser->login.c_str());
    }
iaUser->aliveSent = true;
#endif

aliveSyn6.cash =(int64_t) (iaUser->user->GetProperty().cash.Get() * 1000.0);
if (!stgSettings->GetShowFeeInCash())
    aliveSyn6.cash -= (int64_t)(tf->GetFee() * 1000.0);

#ifdef ARCH_BE
SwapBytes(aliveSyn6.len);
SwapBytes(aliveSyn6.rnd);
SwapBytes(aliveSyn6.cash);
for (int i = 0; i < DIR_NUM; ++i)
    {
    SwapBytes(aliveSyn6.mu[i]);
    SwapBytes(aliveSyn6.md[i]);
    SwapBytes(aliveSyn6.su[i]);
    SwapBytes(aliveSyn6.sd[i]);
    }
#endif

Encrypt(&(iaUser->ctx), (char*)&aliveSyn6, (char*)&aliveSyn6, Min8(sizeof(aliveSyn6))/8);
return Send(sip, iaSettings.GetUserPort(), (char*)&aliveSyn6, Min8(sizeof(aliveSyn6)));
}
//-----------------------------------------------------------------------------
int AUTH_IA::Send_ALIVE_SYN_7(IA_USER * iaUser, uint32_t sip)
{
return Send_ALIVE_SYN_6(iaUser, sip);
}
//-----------------------------------------------------------------------------
int AUTH_IA::Send_ALIVE_SYN_8(IA_USER * iaUser, uint32_t sip)
{
strcpy((char*)aliveSyn8.hdr.magic, IA_ID);
aliveSyn8.hdr.protoVer[0] = 0;
aliveSyn8.hdr.protoVer[1] = 8;

aliveSyn8.len = Min8(sizeof(ALIVE_SYN_8));
aliveSyn8.rnd = iaUser->rnd = static_cast<uint32_t>(random());

strcpy((char*)aliveSyn8.type, "ALIVE_SYN");

for (int i = 0; i < DIR_NUM; i++)
    {
    aliveSyn8.md[i] = iaUser->user->GetProperty().down.Get()[i];
    aliveSyn8.mu[i] = iaUser->user->GetProperty().up.Get()[i];

    aliveSyn8.sd[i] = iaUser->user->GetSessionDownload()[i];
    aliveSyn8.su[i] = iaUser->user->GetSessionUpload()[i];
    }

//TODO
int dn = iaSettings.GetFreeMbShowType();

if (dn < DIR_NUM)
    {
    const TARIFF * tf = iaUser->user->GetTariff();
    double p = tf->GetPriceWithTraffType(aliveSyn8.mu[dn],
                                         aliveSyn8.md[dn],
                                         dn,
                                         stgTime);
    p *= 1024 * 1024;
    if (std::fabs(p) < 1.0e-3)
        {
        snprintf((char*)aliveSyn8.freeMb, IA_FREEMB_LEN, "---");
        }
    else
        {
        double fmb = iaUser->user->GetProperty().freeMb;
        fmb = fmb < 0 ? 0 : fmb;
        snprintf((char*)aliveSyn8.freeMb, IA_FREEMB_LEN, "%.3f", fmb / p);
        }
    }
else
    {
    if (freeMbNone == iaSettings.GetFreeMbShowType())
        {
        aliveSyn8.freeMb[0] = 0;
        }
    else
        {
        double fmb = iaUser->user->GetProperty().freeMb;
        fmb = fmb < 0 ? 0 : fmb;
        snprintf((char*)aliveSyn8.freeMb, IA_FREEMB_LEN, "C%.3f", fmb);
        }
    }

#ifdef IA_DEBUG
if (iaUser->aliveSent)
    {
    printfd(__FILE__, "========= ALIVE_ACK_8 TIMEOUT !!! =========\n");
    }
iaUser->aliveSent = true;
#endif

const TARIFF * tf = iaUser->user->GetTariff();

aliveSyn8.cash =(int64_t) (iaUser->user->GetProperty().cash.Get() * 1000.0);
if (!stgSettings->GetShowFeeInCash())
    aliveSyn8.cash -= (int64_t)(tf->GetFee() * 1000.0);

#ifdef ARCH_BE
SwapBytes(aliveSyn8.len);
SwapBytes(aliveSyn8.rnd);
SwapBytes(aliveSyn8.cash);
SwapBytes(aliveSyn8.status);
for (int i = 0; i < DIR_NUM; ++i)
    {
    SwapBytes(aliveSyn8.mu[i]);
    SwapBytes(aliveSyn8.md[i]);
    SwapBytes(aliveSyn8.su[i]);
    SwapBytes(aliveSyn8.sd[i]);
    }
#endif

Encrypt(&(iaUser->ctx), (char*)&aliveSyn8, (char*)&aliveSyn8, Min8(sizeof(aliveSyn8))/8);
return Send(sip, iaUser->port, (char*)&aliveSyn8, Min8(sizeof(aliveSyn8)));
}
//-----------------------------------------------------------------------------
int AUTH_IA::Send_DISCONN_SYN_ACK_6(IA_USER * iaUser, uint32_t sip)
{
disconnSynAck6.len = Min8(sizeof(DISCONN_SYN_ACK_6));
strcpy((char*)disconnSynAck6.type, "DISCONN_SYN_ACK");
disconnSynAck6.rnd = iaUser->rnd = static_cast<uint32_t>(random());

#ifdef ARCH_BE
SwapBytes(disconnSynAck6.len);
SwapBytes(disconnSynAck6.rnd);
#endif

Encrypt(&iaUser->ctx, (char*)&disconnSynAck6, (char*)&disconnSynAck6, Min8(sizeof(disconnSynAck6))/8);
return Send(sip, iaSettings.GetUserPort(), (char*)&disconnSynAck6, Min8(sizeof(disconnSynAck6)));
}
//-----------------------------------------------------------------------------
int AUTH_IA::Send_DISCONN_SYN_ACK_7(IA_USER * iaUser, uint32_t sip)
{
return Send_DISCONN_SYN_ACK_6(iaUser, sip);
}
//-----------------------------------------------------------------------------
int AUTH_IA::Send_DISCONN_SYN_ACK_8(IA_USER * iaUser, uint32_t sip)
{
strcpy((char*)disconnSynAck8.hdr.magic, IA_ID);
disconnSynAck8.hdr.protoVer[0] = 0;
disconnSynAck8.hdr.protoVer[1] = 8;

disconnSynAck8.len = Min8(sizeof(DISCONN_SYN_ACK_8));
strcpy((char*)disconnSynAck8.type, "DISCONN_SYN_ACK");
disconnSynAck8.rnd = iaUser->rnd = static_cast<uint32_t>(random());

#ifdef ARCH_BE
SwapBytes(disconnSynAck8.len);
SwapBytes(disconnSynAck8.rnd);
#endif

Encrypt(&iaUser->ctx, (char*)&disconnSynAck8, (char*)&disconnSynAck8, Min8(sizeof(disconnSynAck8))/8);
return Send(sip, iaUser->port, (char*)&disconnSynAck8, Min8(sizeof(disconnSynAck8)));
}
//-----------------------------------------------------------------------------
int AUTH_IA::Send_FIN_6(IA_USER * iaUser, uint32_t sip, std::map<uint32_t, IA_USER>::iterator it)
{
fin6.len = Min8(sizeof(FIN_6));
strcpy((char*)fin6.type, "FIN");
strcpy((char*)fin6.ok, "OK");

#ifdef ARCH_BE
SwapBytes(fin6.len);
#endif

Encrypt(&iaUser->ctx, (char*)&fin6, (char*)&fin6, Min8(sizeof(fin6))/8);

users->Unauthorize(iaUser->login, this);

int res = Send(sip, iaSettings.GetUserPort(), (char*)&fin6, Min8(sizeof(fin6)));

ip2user.erase(it);

return res;
}
//-----------------------------------------------------------------------------
int AUTH_IA::Send_FIN_7(IA_USER * iaUser, uint32_t sip, std::map<uint32_t, IA_USER>::iterator it)
{
return Send_FIN_6(iaUser, sip, it);
}
//-----------------------------------------------------------------------------
int AUTH_IA::Send_FIN_8(IA_USER * iaUser, uint32_t sip, std::map<uint32_t, IA_USER>::iterator it)
{
strcpy((char*)fin8.hdr.magic, IA_ID);
fin8.hdr.protoVer[0] = 0;
fin8.hdr.protoVer[1] = 8;

fin8.len = Min8(sizeof(FIN_8));
strcpy((char*)fin8.type, "FIN");
strcpy((char*)fin8.ok, "OK");

#ifdef ARCH_BE
SwapBytes(fin8.len);
#endif

Encrypt(&iaUser->ctx, (char*)&fin8, (char*)&fin8, Min8(sizeof(fin8))/8);

users->Unauthorize(iaUser->login, this);

int res = Send(sip, iaUser->port, (char*)&fin8, Min8(sizeof(fin8)));

ip2user.erase(it);

return res;
}
namespace
{
//-----------------------------------------------------------------------------
inline
void InitEncrypt(BLOWFISH_CTX * ctx, const std::string & password)
{
unsigned char keyL[PASSWD_LEN];
memset(keyL, 0, PASSWD_LEN);
strncpy((char *)keyL, password.c_str(), PASSWD_LEN);
Blowfish_Init(ctx, keyL, PASSWD_LEN);
}
//-----------------------------------------------------------------------------
inline
void Decrypt(BLOWFISH_CTX * ctx, void * dst, const void * src, size_t len8)
{
for (size_t i = 0; i < len8; i++)
    DecodeString(static_cast<char *>(dst) + i * 8, static_cast<const char *>(src) + i * 8, ctx);
}
//-----------------------------------------------------------------------------
inline
void Encrypt(BLOWFISH_CTX * ctx, void * dst, const void * src, size_t len8)
{
for (size_t i = 0; i < len8; i++)
    EncodeString(static_cast<char *>(dst) + i * 8, static_cast<const char *>(src) + i * 8, ctx);
}
//-----------------------------------------------------------------------------
}
