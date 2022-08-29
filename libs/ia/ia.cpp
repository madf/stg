/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 1, or (at your option)
** any later version.

** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.

** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
 $Author: faust $
 $Revision: 1.15 $
 $Date: 2010/04/16 11:28:03 $
*/

/*
* Author :
* Boris Mikhailenko <stg34@stargazer.dp.ua>
* Maxim Mamontov <faust@stargazer.dp.ua>
* Andrey Rakhmanov <andrey_rakhmanov@yahoo.com> - bugfixes.
*/

//---------------------------------------------------------------------------

#include "stg/common.h"
#include "stg/ia.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <winbase.h>
#include <winnt.h>
#else
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <csignal>
#endif

#define IA_NONE            (0)
#define IA_CONNECT         (1)
#define IA_DISCONNECT      (2)

#define IA_DEBUGPROTO   1

#define IA_ID "00100"
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifndef WIN32
#include <sys/time.h>
//---------------------------------------------------------------------------
long GetTickCount()
{
struct timeval tv;
gettimeofday(&tv, NULL);
return tv.tv_sec*1000 + tv.tv_usec/1000;
}
#endif
//---------------------------------------------------------------------------

namespace
{

bool HostNameToIP(const std::string & hostName, uint32_t & ip)
{
ip = inet_addr(hostName.c_str());
if (ip == INADDR_NONE)
    {
    hostent * phe = gethostbyname(hostName.c_str());
    if (phe)
        {
        ip = *reinterpret_cast<uint32_t*>(phe->h_addr_list[0]);
        }
    else
        {
        return false;
        }
    }

return true;
}

}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
IA_CLIENT_PROT::IA_CLIENT_PROT(const std::string & sn, unsigned short p,
                               const std::string & ln, uint16_t lp)
    : m_action(IA_NONE),
      m_phase(1),
      m_phaseTime(0),
      m_codeError(0),
      m_isNetPrepared(false),
      m_proxyMode(false),
      m_serverName(sn),
      m_port(p),
      m_ip(0),
      m_localName(ln),
      m_localPort(lp),
      m_firstConnect(true),
      m_reconnect(0),
      m_sockr(0),
      m_protNum(0),
      m_userTimeout(60),
      m_aliveTimeout(5),
      m_rnd(0),
      m_pStatusChangedCb(NULL),
      m_pStatChangedCb(NULL),
      m_pInfoCb(NULL),
      m_pErrorCb(NULL),
      m_pDirNameCb(NULL),
      m_statusChangedCbData(NULL),
      m_statChangedCbData(NULL),
      m_infoCbData(NULL),
      m_errorCbData(NULL),
      m_dirNameCbData(NULL),
      m_connSyn8(NULL),
      m_connSynAck8(NULL),
      m_connAck8(NULL),
      m_aliveSyn8(NULL),
      m_aliveAck8(NULL),
      m_disconnSyn8(NULL),
      m_disconnSynAck8(NULL),
      m_disconnAck8(NULL),
      m_info(NULL)
{
memset(&m_stat, 0, sizeof(m_stat));

#ifdef WIN32
WSAStartup(MAKEWORD(2, 0), &m_wsaData);
#endif

m_packetTypes["CONN_SYN"] = CONN_SYN_N;
m_packetTypes["CONN_SYN_ACK"] = CONN_SYN_ACK_N;
m_packetTypes["CONN_ACK"] = CONN_ACK_N;
m_packetTypes["ALIVE_SYN"] = ALIVE_SYN_N;
m_packetTypes["ALIVE_ACK"] = ALIVE_ACK_N;
m_packetTypes["DISCONN_SYN"] = DISCONN_SYN_N;
m_packetTypes["DISCONN_SYN_ACK"] = DISCONN_SYN_ACK_N;
m_packetTypes["DISCONN_ACK"] = DISCONN_ACK_N;
m_packetTypes["FIN"] = FIN_N;
m_packetTypes["ERR"] = ERROR_N;
m_packetTypes["INFO"] = INFO_N;
m_packetTypes["INFO_7"] = INFO_7_N;
m_packetTypes["INFO_8"] = INFO_8_N;

char key[IA_PASSWD_LEN];
memset(key, 0, IA_PASSWD_LEN);
strncpy(key, "pr7Hhen", 8);
Blowfish_Init(&m_ctxHdr, key, IA_PASSWD_LEN);

memset(key, 0, IA_PASSWD_LEN);
Blowfish_Init(&m_ctxPass, key, IA_PASSWD_LEN);

for (size_t i = 0; i < DIR_NUM; ++i)
    m_selectedDirs[i] = false;

m_servAddr.sin_family = AF_INET;
m_servAddr.sin_port = htons(m_port);
m_servAddr.sin_addr.s_addr = m_ip;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::PrepareNet()
{
/*struct hostent * phe;
unsigned long ip;

ip = inet_addr(serverName.c_str());
if (ip == INADDR_NONE)
    {
    phe = gethostbyname(serverName.c_str());
    if (phe)
        {
        ip = *((unsigned long*)phe->h_addr_list[0]);
        }
    else
        {
        strError = string("Unknown host ") + "\'" + serverName + "\'";
        codeError = IA_GETHOSTBYNAME_ERROR;
        if (pErrorCb != NULL)
            pErrorCb(strError, IA_GETHOSTBYNAME_ERROR, errorCbData);
        }
    }*/

if (!HostNameToIP(m_serverName, m_ip))
    {
    m_ip = 0;
    m_strError = std::string("Unknown host ") + "\'" + m_serverName + "\'";
    m_codeError = IA_GETHOSTBYNAME_ERROR;
    if (m_pErrorCb != NULL)
        m_pErrorCb(m_strError, IA_GETHOSTBYNAME_ERROR, m_errorCbData);
    return;
    }

#ifndef WIN32
close(m_sockr);
#else
closesocket(m_sockr);
#endif

m_sockr = socket(AF_INET, SOCK_DGRAM, 0);

struct sockaddr_in  localAddrR;
localAddrR.sin_family = AF_INET;

if (m_localPort)
    localAddrR.sin_port = htons(m_localPort);
else
    localAddrR.sin_port = htons(m_port);

if (!m_localName.empty())
    {
    if (!HostNameToIP(m_localName, m_localIP))
        {
        m_strError = std::string("Unknown host ") + "\'" + m_serverName + "\'";
        m_codeError = IA_GETHOSTBYNAME_ERROR;
        if (m_pErrorCb != NULL)
            m_pErrorCb(m_strError, IA_GETHOSTBYNAME_ERROR, m_errorCbData);
        m_localIP = INADDR_ANY;
        }
    }
else
    {
    m_localIP = INADDR_ANY;
    }

localAddrR.sin_addr.s_addr = m_localIP;

m_servAddr.sin_family = AF_INET;
m_servAddr.sin_port = htons(m_port);
m_servAddr.sin_addr.s_addr = m_ip;

int res = bind(m_sockr, reinterpret_cast<sockaddr*>(&localAddrR), sizeof(localAddrR));
if (res == -1)
    {
    m_strError = "bind error";
    m_codeError = IA_BIND_ERROR;
    if (m_pErrorCb != NULL)
        m_pErrorCb(m_strError, IA_BIND_ERROR, m_errorCbData);
    return;
    }

#ifdef WIN32
unsigned long arg = 1;
ioctlsocket(m_sockr, FIONBIO, &arg);
#else
if (0 != fcntl(m_sockr, F_SETFL, O_NONBLOCK))
    {
    m_strError = "fcntl error";
    m_codeError = IA_FCNTL_ERROR;
    if (m_pErrorCb != NULL)
        m_pErrorCb(m_strError, IA_FCNTL_ERROR, m_errorCbData);
    }
#endif

}
//---------------------------------------------------------------------------
IA_CLIENT_PROT::~IA_CLIENT_PROT()
{
#ifndef WIN32
close(m_sockr);
#else
closesocket(m_sockr);
WSACleanup();
#endif
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::DeterminatePacketType(const char * buffer)
{
std::map<std::string, int>::iterator pi;
pi = m_packetTypes.find(buffer);
if (pi == m_packetTypes.end())
    {
    return -1;
    }
else
    {
    return pi->second;
    }
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::FillHdr8(char* buffer, unsigned long)
{
strncpy(buffer, IA_ID, 6);
buffer[IA_MAGIC_LEN] = 0;
buffer[IA_MAGIC_LEN + 1] = IA_PROTO_VER;
strncpy(buffer + sizeof(HDR_8), m_login.c_str(), IA_LOGIN_LEN);
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Send(char * buffer, int len)
{
if (!m_isNetPrepared)
    {
    PrepareNet();
    m_isNetPrepared = true;
    }

int db = sizeof(HDR_8);
EncryptString(buffer + db, buffer + db, IA_LOGIN_LEN, &m_ctxHdr);

db += IA_LOGIN_LEN;
int encLen = (len - sizeof(HDR_8) - IA_LOGIN_LEN);
EncryptString(buffer + db, buffer + db, encLen, &m_ctxPass);

return sendto(m_sockr, buffer, len, 0, reinterpret_cast<sockaddr*>(&m_servAddr), sizeof(m_servAddr));
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Recv(char * buffer, int len)
{
#ifdef WIN32
int fromLen;
#else
socklen_t fromLen;
#endif

struct sockaddr_in addr;
fromLen = sizeof(addr);
int res = recvfrom(m_sockr, buffer, len, 0, reinterpret_cast<sockaddr*>(&addr), &fromLen);

if (res == -1)
    return res;

if (strcmp(buffer + 4 + sizeof(HDR_8), "ERR"))
    DecryptString(buffer, buffer, len, &m_ctxPass);

return 0;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::NetSend(int n)
{
char buffer[2048];
int msgLen;

memset(buffer, 0, 2048);

switch (n)
    {
    case CONN_SYN_N:
        msgLen = Prepare_CONN_SYN_8(buffer);
        break;

    case CONN_ACK_N:
        msgLen = Prepare_CONN_ACK_8(buffer);
        break;

    case ALIVE_ACK_N:
        msgLen = Prepare_ALIVE_ACK_8(buffer);
        break;

    case DISCONN_SYN_N:
        msgLen = Prepare_DISCONN_SYN_8(buffer);
        break;

    case DISCONN_ACK_N:
        msgLen = Prepare_DISCONN_ACK_8(buffer);
        break;

    default:
        return -1;
    }

FillHdr8(buffer, 0);
Send(buffer, msgLen);

return 0;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::NetRecv()
{
char buffer[2048];

if (Recv(buffer, sizeof(buffer)) < 0)
    return -1;

char packetName[20];
strncpy(packetName, buffer + 12, sizeof(packetName));
packetName[sizeof(packetName) - 1] = 0;
int pn = DeterminatePacketType(buffer + 12);

int ret;
switch (pn)
    {
    case CONN_SYN_ACK_N:
        ret = Process_CONN_SYN_ACK_8(buffer);
        break;

    case ALIVE_SYN_N:
        ret = Process_ALIVE_SYN_8(buffer);
        break;

    case DISCONN_SYN_ACK_N:
        ret = Process_DISCONN_SYN_ACK_8(buffer);
        break;

    case FIN_N:
        ret = Process_FIN_8(buffer);
        break;

    case INFO_8_N:
        ret = Process_INFO_8(buffer);
        break;

    case ERROR_N:
        ret = Process_ERROR(buffer);
        break;

    default:
        ret = -1;
    }
return ret;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::Start()
{
    m_thread = std::jthread([this](auto token){ Run(std::move(token)); });
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::Stop()
{
    if (m_thread.joinable())
        m_thread.request_stop();
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::Run(std::stop_token token)
{
    while (!token.stop_requested())
    {
        NetRecv();

        switch (m_phase)
        {
            case 1:
                if (m_action == IA_CONNECT)
                {
                    m_action = IA_NONE;
                    NetSend(CONN_SYN_N);
                    m_phase = 2;
                    m_phaseTime = GetTickCount();
                }
                if (m_reconnect && !m_firstConnect)
                {
                    m_action = IA_CONNECT;
                }
                break;

            case 2:
                if (static_cast<int>(GetTickCount() - m_phaseTime)/1000 > m_aliveTimeout)
                {
                    m_phase = 1;
                    m_phaseTime = GetTickCount();
                    if (m_pStatusChangedCb != NULL)
                        m_pStatusChangedCb(0, m_statusChangedCbData);
                }

                if (m_action == IA_DISCONNECT)
                {
                    m_action = IA_NONE;
                    NetSend(DISCONN_SYN_N);
                    m_phase = 4;
                    m_phaseTime = GetTickCount();
                }

                break;

            case 3:
                if (static_cast<int>(GetTickCount() - m_phaseTime)/1000 > m_userTimeout)
                {
                    m_phase = 1;
                    m_phaseTime = GetTickCount();
                    if (m_pStatusChangedCb != NULL)
                        m_pStatusChangedCb(0, m_statusChangedCbData);
                    m_firstConnect = false;
                }

                if (m_action == IA_DISCONNECT)
                {
                    m_action = IA_NONE;
                    NetSend(DISCONN_SYN_N);
                    m_phase = 4;
                    m_phaseTime = GetTickCount();
                }

                break;

            case 4:
                if (static_cast<int>(GetTickCount() - m_phaseTime)/1000 > m_aliveTimeout)
                {
                    m_phase=1;
                    m_phaseTime = GetTickCount();
                    if (m_pStatusChangedCb != NULL)
                        m_pStatusChangedCb(0, m_statusChangedCbData);
                }

                if (m_action == IA_CONNECT)
                {
                    m_action = IA_NONE;
                    NetSend(CONN_SYN_N);
                    m_phase = 2;
                    m_phaseTime = GetTickCount();
                }

                break;

            case 5:
                if (static_cast<int>(GetTickCount() - m_phaseTime)/1000 > m_aliveTimeout)
                {
                    m_phase = 1;
                    m_phaseTime = GetTickCount();
                    if (m_pStatusChangedCb != NULL)
                        m_pStatusChangedCb(0, m_statusChangedCbData);
                }

                if (m_action == IA_CONNECT)
                {
                    m_action = IA_NONE;
                    NetSend(CONN_SYN_N);
                    m_phase = 2;
                    m_phaseTime = GetTickCount();
                }

                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::GetStat(LOADSTAT * ls)
{
memcpy(ls, &m_stat, sizeof(m_stat));
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetServer(const std::string & sn, unsigned short p)
{
m_serverName = sn;
m_port = p;
PrepareNet();
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetLogin(const std::string & l)
{
m_login = l;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetPassword(const std::string & p)
{
m_password = p;

char keyL[IA_PASSWD_LEN];
memset(keyL, 0, IA_PASSWD_LEN);
strncpy(keyL, m_password.c_str(), IA_PASSWD_LEN);
Blowfish_Init(&m_ctxPass, keyL, IA_PASSWD_LEN);
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetEnabledDirs(const bool * selectedDirs)
{
memcpy(m_selectedDirs, selectedDirs, sizeof(bool) * DIR_NUM);
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Connect()
{
m_action = IA_CONNECT;
return 0;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Disconnect()
{
m_firstConnect = true;
m_action = IA_DISCONNECT;
return 0;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::GetStrError(std::string * error) const
{
int ret = m_codeError;
*error = m_strError;
m_strError = "";
m_codeError = 0;
return ret;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Process_CONN_SYN_ACK_8(const void* buffer)
{
std::vector<std::string> dirNames;
m_connSynAck8 = static_cast<const CONN_SYN_ACK_8*>(buffer);

#ifdef ARCH_BE
SwapBytes(m_connSynAck8->len);
SwapBytes(m_connSynAck8->rnd);
SwapBytes(m_connSynAck8->userTimeOut);
SwapBytes(m_connSynAck8->aliveDelay);
#endif

m_rnd = m_connSynAck8->rnd;
m_userTimeout = m_connSynAck8->userTimeOut;
m_aliveTimeout = m_connSynAck8->aliveDelay;

for (int i = 0; i < DIR_NUM; i++)
    dirNames.push_back(reinterpret_cast<const char*>(m_connSynAck8->dirName[i]));

if (m_pDirNameCb != NULL)
    m_pDirNameCb(dirNames, m_dirNameCbData);

NetSend(CONN_ACK_N);
m_phase = 3;
m_phaseTime = GetTickCount();

return CONN_SYN_ACK_N;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Process_ALIVE_SYN_8(const void* buffer)
{
m_aliveSyn8 = static_cast<const ALIVE_SYN_8*>(buffer);

#ifdef ARCH_BE
SwapBytes(m_aliveSyn8->len);
SwapBytes(m_aliveSyn8->rnd);
SwapBytes(m_aliveSyn8->cash);
SwapBytes(m_aliveSyn8->status);
for (int i = 0; i < DIR_NUM; ++i)
    {
    SwapBytes(m_aliveSyn8->mu[i]);
    SwapBytes(m_aliveSyn8->md[i]);
    SwapBytes(m_aliveSyn8->su[i]);
    SwapBytes(m_aliveSyn8->sd[i]);
    }
#endif

m_rnd = m_aliveSyn8->rnd;
memcpy(&m_stat, m_aliveSyn8->mu, sizeof(m_stat));

if (m_pStatChangedCb != NULL)
    m_pStatChangedCb(m_stat, m_statChangedCbData);

if (m_pStatusChangedCb != NULL)
    m_pStatusChangedCb(1, m_statusChangedCbData);
NetSend(ALIVE_ACK_N);
m_phaseTime = GetTickCount();

return ALIVE_SYN_N;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Process_DISCONN_SYN_ACK_8(const void* buffer)
{
m_disconnSynAck8 = static_cast<const DISCONN_SYN_ACK_8*>(buffer);

#ifdef ARCH_BE
SwapBytes(m_disconnSynAck8->len);
SwapBytes(m_disconnSynAck8->rnd);
#endif

m_rnd = m_disconnSynAck8->rnd;

NetSend(DISCONN_ACK_N);
m_phase = 5;
m_phaseTime = GetTickCount();

return DISCONN_SYN_ACK_N;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Process_FIN_8(const void*)
{
m_phase = 1;
m_phaseTime = GetTickCount();
if (m_pStatusChangedCb != NULL)
    m_pStatusChangedCb(0, m_statusChangedCbData);

return FIN_N;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Process_INFO_8(const void* buffer)
{
m_info = static_cast<const INFO_8*>(buffer);

#ifdef ARCH_BE
SwapBytes(m_info->len);
SwapBytes(m_info->sendTime);
#endif

if (m_pInfoCb != NULL)
    m_pInfoCb(reinterpret_cast<const char*>(m_info->text), m_info->infoType, m_info->showTime, m_info->sendTime, m_infoCbData);
return INFO_8_N;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Process_ERROR(const void* buffer)
{
ERR_8 err;
memcpy(&err, buffer, sizeof(err));

#ifdef ARCH_BE
SwapBytes(err.len);
#endif

KOIToWin(reinterpret_cast<const char*>(err.text), &m_messageText);
if (m_pErrorCb != NULL)
    m_pErrorCb(m_messageText, IA_SERVER_ERROR, m_errorCbData);
m_phase = 1;
m_phaseTime = GetTickCount();
m_codeError = IA_SERVER_ERROR;

return ERROR_N;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Prepare_CONN_SYN_8(void* buffer)
{
m_connSyn8 = static_cast<CONN_SYN_8*>(buffer);

assert(sizeof(CONN_SYN_8) == Min8(sizeof(CONN_SYN_8)) && "CONN_SYN_8 is not aligned to 8 bytes");

m_connSyn8->len = sizeof(CONN_SYN_8);

#ifdef ARCH_BE
SwapBytes(m_connSyn8->len);
#endif

strncpy(reinterpret_cast<char*>(m_connSyn8->type), "CONN_SYN", IA_MAX_TYPE_LEN);
strncpy(reinterpret_cast<char*>(m_connSyn8->login), m_login.c_str(), IA_LOGIN_LEN);
m_connSyn8->dirs = 0;
for (int i = 0; i < DIR_NUM; i++)
    m_connSyn8->dirs |= (m_selectedDirs[i] << i);
return sizeof(CONN_SYN_8);
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Prepare_CONN_ACK_8(void* buffer)
{
m_connAck8 = static_cast<CONN_ACK_8*>(buffer);

assert(sizeof(CONN_ACK_8) == Min8(sizeof(CONN_ACK_8)) && "CONN_ACK_8 is not aligned to 8 bytes");

m_connAck8->len = sizeof(CONN_ACK_8);
strncpy(reinterpret_cast<char*>(m_connAck8->loginS), m_login.c_str(), IA_LOGIN_LEN);
strncpy(reinterpret_cast<char*>(m_connAck8->type), "CONN_ACK", IA_MAX_TYPE_LEN);
m_rnd++;
m_connAck8->rnd = m_rnd;

#ifdef ARCH_BE
SwapBytes(m_connAck8->len);
SwapBytes(m_connAck8->rnd);
#endif

return sizeof(CONN_ACK_8);
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Prepare_ALIVE_ACK_8(void* buffer)
{
m_aliveAck8 = static_cast<ALIVE_ACK_8*>(buffer);

assert(Min8(sizeof(ALIVE_ACK_8)) == sizeof(ALIVE_ACK_8) && "ALIVE_ACK_8 is not aligned to 8 bytes");

m_aliveAck8->len = sizeof(ALIVE_ACK_8);
strncpy(reinterpret_cast<char*>(m_aliveAck8->loginS), m_login.c_str(), IA_LOGIN_LEN);
strncpy(reinterpret_cast<char*>(m_aliveAck8->type), "ALIVE_ACK", IA_MAX_TYPE_LEN);
m_aliveAck8->rnd = ++m_rnd;

#ifdef ARCH_BE
SwapBytes(m_aliveAck8->len);
SwapBytes(m_aliveAck8->rnd);
#endif

return sizeof(ALIVE_ACK_8);
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Prepare_DISCONN_SYN_8(void* buffer)
{
m_disconnSyn8 = static_cast<DISCONN_SYN_8*>(buffer);

assert(Min8(sizeof(DISCONN_SYN_8)) == sizeof(DISCONN_SYN_8) && "DISCONN_SYN_8 is not aligned to 8 bytes");

m_disconnSyn8->len = sizeof(DISCONN_SYN_8);

#ifdef ARCH_BE
SwapBytes(m_disconnSyn8->len);
#endif

strncpy(reinterpret_cast<char*>(m_disconnSyn8->loginS), m_login.c_str(), IA_LOGIN_LEN);
strncpy(reinterpret_cast<char*>(m_disconnSyn8->type), "DISCONN_SYN", IA_MAX_TYPE_LEN);
strncpy(reinterpret_cast<char*>(m_disconnSyn8->login), m_login.c_str(), IA_LOGIN_LEN);
return sizeof(DISCONN_SYN_8);
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Prepare_DISCONN_ACK_8(void* buffer)
{
m_disconnAck8 = static_cast<DISCONN_ACK_8*>(buffer);

assert(Min8(sizeof(DISCONN_ACK_8)) == sizeof(DISCONN_ACK_8) && "DISCONN_ACK_8 is not aligned to 8 bytes");

m_disconnAck8->len = Min8(sizeof(DISCONN_ACK_8));
m_disconnAck8->rnd = m_rnd + 1;

#ifdef ARCH_BE
SwapBytes(m_disconnAck8->len);
SwapBytes(m_disconnAck8->rnd);
#endif

strncpy(reinterpret_cast<char*>(m_disconnAck8->loginS), m_login.c_str(), IA_LOGIN_LEN);
strncpy(reinterpret_cast<char*>(m_disconnAck8->type), "DISCONN_ACK", IA_MAX_TYPE_LEN);
return Min8(sizeof(DISCONN_ACK_8));
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetStatusChangedCb(tpStatusChangedCb p, void * data)
{
m_pStatusChangedCb = p;
m_statusChangedCbData = data;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetStatChangedCb(tpStatChangedCb p, void * data)
{
m_pStatChangedCb = p;
m_statChangedCbData = data;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetInfoCb(tpCallBackInfoFn p, void * data)
{
m_pInfoCb = p;
m_infoCbData = data;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetDirNameCb(tpCallBackDirNameFn p, void * data)
{
m_pDirNameCb = p;
m_dirNameCbData = data;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetErrorCb(tpCallBackErrorFn p, void * data)
{
m_pErrorCb = p;
m_errorCbData = data;
}
//---------------------------------------------------------------------------
