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
#endif

#include "stg/common.h"
#include "ia.h"

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
void * RunL(void * data)
{

IA_CLIENT_PROT * c = (IA_CLIENT_PROT *)data;
static int a = 0;

if (a == 0)
    {
    usleep(50000);
    a = 1;
    }

while (c->GetNonstop())
    {
    c->Run();
    }
return NULL;
}
//---------------------------------------------------------------------------
void Sleep(int ms)
{
usleep(ms * 1000);
}
//---------------------------------------------------------------------------
long GetTickCount()
{
struct timeval tv;
gettimeofday(&tv, NULL);
return tv.tv_sec*1000 + tv.tv_usec/1000;
}
#else
//---------------------------------------------------------------------------
unsigned long WINAPI RunW(void * data)
{
IA_CLIENT_PROT * c = (IA_CLIENT_PROT *)data;
while (c->GetNonstop())
    c->Run();
return 0;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
IA_CLIENT_PROT::IA_CLIENT_PROT(const string & sn, unsigned short p, uint16_t localPort)
    : action(IA_NONE),
      phase(1),
      phaseTime(0),
      codeError(0),
      nonstop(false),
      isNetPrepared(false),
      proxyMode(false),
      serverName(sn),
      port(p),
      ip(0),
      localPort(localPort),
      firstConnect(true),
      reconnect(0),
      sockr(0),
      protNum(0),
      userTimeout(60),
      aliveTimeout(5),
      rnd(0),
      pStatusChangedCb(NULL),
      pStatChangedCb(NULL),
      pInfoCb(NULL),
      pErrorCb(NULL),
      pDirNameCb(NULL),
      statusChangedCbData(NULL),
      statChangedCbData(NULL),
      infoCbData(NULL),
      errorCbData(NULL),
      dirNameCbData(NULL),
      connSyn8(NULL),
      connSynAck8(NULL),
      connAck8(NULL),
      aliveSyn8(NULL),
      aliveAck8(NULL),
      disconnSyn8(NULL),
      disconnSynAck8(NULL),
      disconnAck8(NULL),
      info(NULL)
{
memset(&stat, 0, sizeof(stat));

#ifdef WIN32
WSAStartup(MAKEWORD(2, 0), &wsaData);
#endif

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
packetTypes["INFO"] = INFO_N;
packetTypes["INFO_7"] = INFO_7_N;
packetTypes["INFO_8"] = INFO_8_N;

unsigned char key[IA_PASSWD_LEN];
memset(key, 0, IA_PASSWD_LEN);
strncpy((char *)key, "pr7Hhen", 8);
Blowfish_Init(&ctxHdr, key, IA_PASSWD_LEN);

memset(key, 0, IA_PASSWD_LEN);
Blowfish_Init(&ctxPass, key, IA_PASSWD_LEN);

for (size_t i = 0; i < DIR_NUM; ++i)
    {
    selectedDirs[i] = false;
    }

servAddr.sin_family = AF_INET;
servAddr.sin_port = htons(port);
servAddr.sin_addr.s_addr = ip;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::PrepareNet()
{
struct hostent * phe;
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
    }

#ifndef WIN32
close(sockr);
#else
closesocket(sockr);
#endif

sockr = socket(AF_INET, SOCK_DGRAM, 0);

struct sockaddr_in  localAddrR;
localAddrR.sin_family = AF_INET;

if (localPort)
    localAddrR.sin_port = htons(localPort);
else
    localAddrR.sin_port = htons(port);
localAddrR.sin_addr.s_addr = inet_addr("0.0.0.0");

servAddr.sin_family = AF_INET;
servAddr.sin_port = htons(port);
servAddr.sin_addr.s_addr = ip;

int res = bind(sockr, (struct sockaddr*)&localAddrR, sizeof(localAddrR));
if (res == -1)
    {
    strError = "bind error";
    codeError = IA_BIND_ERROR;
    if (pErrorCb != NULL)
        pErrorCb(strError, IA_BIND_ERROR, errorCbData);
    return;
    }

#ifdef WIN32
unsigned long arg = 1;
res = ioctlsocket(sockr, FIONBIO, &arg);
#else
if (0 != fcntl(sockr, F_SETFL, O_NONBLOCK))
    {
    strError = "fcntl error";
    codeError = IA_FCNTL_ERROR;
    if (pErrorCb != NULL)
        pErrorCb(strError, IA_FCNTL_ERROR, errorCbData);
    }
#endif

}
//---------------------------------------------------------------------------
IA_CLIENT_PROT::~IA_CLIENT_PROT()
{
#ifndef WIN32
close(sockr);
#else
closesocket(sockr);
WSACleanup();
#endif
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::DeterminatePacketType(const char * buffer)
{
map<string, int>::iterator pi;
pi = packetTypes.find(buffer);
if (pi == packetTypes.end())
    {
    return -1;
    }
else
    {
    return pi->second;
    }
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::FillHdr8(char * buffer, unsigned long)
{
strncpy(buffer, IA_ID, 6);
buffer[IA_MAGIC_LEN] = 0;
buffer[IA_MAGIC_LEN + 1] = IA_PROTO_VER;
strncpy(buffer + sizeof(HDR_8), login.c_str(), IA_LOGIN_LEN);
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Send(char * buffer, int len)
{
if (!isNetPrepared)
    {
    PrepareNet();
    isNetPrepared = true;
    }

int db = sizeof(HDR_8);
for (int i = 0; i < IA_LOGIN_LEN/8; i++)
    {
    Blowfish_Encrypt(&ctxHdr, (uint32_t*)(buffer + db + i*8), (uint32_t*)(buffer + db + i*8 + 4));
    }

db += IA_LOGIN_LEN;
int encLen = (len - sizeof(HDR_8) - IA_LOGIN_LEN)/8;
for (int i = 0; i < encLen; i++)
    {
    Blowfish_Encrypt(&ctxPass, (uint32_t*)(buffer + db), (uint32_t*)(buffer + db + 4));
    db += 8;
    }

return sendto(sockr, buffer, len, 0, (struct sockaddr*)&servAddr, sizeof(servAddr));
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
int res = recvfrom(sockr, buffer, len, 0, (struct sockaddr*)&addr, &fromLen);

if (res == -1)
    return res;

if (strcmp(buffer + 4 + sizeof(HDR_8), "ERR"))
    {
    for (int i = 0; i < len/8; i++)
        Blowfish_Decrypt(&ctxPass, (uint32_t*)(buffer + i*8), (uint32_t*)(buffer + i*8 + 4));
    }

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
nonstop = true;
#ifdef WIN32
unsigned long pt;
CreateThread(NULL, 16384, RunW, this, 0, &pt);
#else
pthread_create(&thread, NULL, RunL, this);
#endif
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::Stop()
{
nonstop = false;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::Run()
{
NetRecv();

switch (phase)
    {
    case 1:
        if (action == IA_CONNECT)
            {
            action = IA_NONE;
            NetSend(CONN_SYN_N);
            phase = 2;
            phaseTime = GetTickCount();
            }
        if (reconnect && !firstConnect)
            {
            action = IA_CONNECT;
            }
        break;

    case 2:
        if ((int)(GetTickCount() - phaseTime)/1000 > aliveTimeout)
            {
            phase = 1;
            phaseTime = GetTickCount();
            if (pStatusChangedCb != NULL)
                pStatusChangedCb(0, statusChangedCbData);
            }

        if (action == IA_DISCONNECT)
            {
            action = IA_NONE;
            NetSend(DISCONN_SYN_N);
            phase = 4;
            phaseTime = GetTickCount();
            }

        break;

    case 3:
        if ((int)(GetTickCount() - phaseTime)/1000 > userTimeout)
            {
            phase = 1;
            phaseTime = GetTickCount();
            if (pStatusChangedCb != NULL)
                pStatusChangedCb(0, statusChangedCbData);
            firstConnect = false;
            }

        if (action == IA_DISCONNECT)
            {
            action = IA_NONE;
            NetSend(DISCONN_SYN_N);
            phase = 4;
            phaseTime = GetTickCount();
            }

        break;

    case 4:
        if ((int)(GetTickCount() - phaseTime)/1000 > aliveTimeout)
            {
            phase=1;
            phaseTime = GetTickCount();
            if (pStatusChangedCb != NULL)
                pStatusChangedCb(0, statusChangedCbData);
            }

        if (action == IA_CONNECT)
            {
            action = IA_NONE;
            NetSend(CONN_SYN_N);
            phase = 2;
            phaseTime = GetTickCount();
            }

        break;

    case 5:
        if ((int)(GetTickCount() - phaseTime)/1000 > aliveTimeout)
            {
            phase = 1;
            phaseTime = GetTickCount();
            if (pStatusChangedCb != NULL)
                pStatusChangedCb(0, statusChangedCbData);
            }

        if (action == IA_CONNECT)
            {
            action = IA_NONE;
            NetSend(CONN_SYN_N);
            phase = 2;
            phaseTime = GetTickCount();
            }

        break;
    }
Sleep(20);
return;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::GetStat(LOADSTAT * ls)
{
memcpy(ls, &stat, sizeof(stat));
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetServer(const string & sn, unsigned short p)
{
serverName = sn;
port = p;
PrepareNet();
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetLogin(const string & l)
{
login = l;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetPassword(const string & p)
{
password = p;

unsigned char keyL[IA_PASSWD_LEN];
memset(keyL, 0, IA_PASSWD_LEN);
strncpy((char *)keyL, password.c_str(), IA_PASSWD_LEN);
Blowfish_Init(&ctxPass, keyL, IA_PASSWD_LEN);
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetEnabledDirs(const bool * selectedDirs)
{
memcpy(IA_CLIENT_PROT::selectedDirs, selectedDirs, sizeof(bool) * DIR_NUM);
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Connect()
{
action = IA_CONNECT;
return 0;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Disconnect()
{
firstConnect = true;
action = IA_DISCONNECT;
return 0;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::GetStrError(string * error) const
{
int ret = codeError;
*error = strError;
strError = "";
codeError = 0;
return ret;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Process_CONN_SYN_ACK_8(const char * buffer)
{
vector<string> dirNames;
connSynAck8 = (CONN_SYN_ACK_8*)buffer;

#ifdef ARCH_BE
SwapBytes(connSynAck8->len);
SwapBytes(connSynAck8->rnd);
SwapBytes(connSynAck8->userTimeOut);
SwapBytes(connSynAck8->aliveDelay);
#endif

rnd = connSynAck8->rnd;
userTimeout = connSynAck8->userTimeOut;
aliveTimeout = connSynAck8->aliveDelay;

for (int i = 0; i < DIR_NUM; i++)
    {
    dirNames.push_back((const char*)connSynAck8->dirName[i]);
    }

if (pDirNameCb != NULL)
    pDirNameCb(dirNames, dirNameCbData);

NetSend(CONN_ACK_N);
phase = 3;
phaseTime = GetTickCount();

return CONN_SYN_ACK_N;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Process_ALIVE_SYN_8(const char * buffer)
{
aliveSyn8 = (ALIVE_SYN_8*)buffer;

#ifdef ARCH_BE
SwapBytes(aliveSyn8->len);
SwapBytes(aliveSyn8->rnd);
SwapBytes(aliveSyn8->cash);
SwapBytes(aliveSyn8->status);
for (int i = 0; i < DIR_NUM; ++i)
    {
    SwapBytes(aliveSyn8->mu[i]);
    SwapBytes(aliveSyn8->md[i]);
    SwapBytes(aliveSyn8->su[i]);
    SwapBytes(aliveSyn8->sd[i]);
    }
#endif

rnd = aliveSyn8->rnd;
memcpy(&stat, (char*)aliveSyn8->mu, sizeof(stat));

if (pStatChangedCb != NULL)
    pStatChangedCb(stat, statChangedCbData);

if (pStatusChangedCb != NULL)
    pStatusChangedCb(1, statusChangedCbData);
NetSend(ALIVE_ACK_N);
phaseTime = GetTickCount();

return ALIVE_SYN_N;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Process_DISCONN_SYN_ACK_8(const char * buffer)
{
disconnSynAck8 = (DISCONN_SYN_ACK_8*)buffer;

#ifdef ARCH_BE
SwapBytes(disconnSynAck8->len);
SwapBytes(disconnSynAck8->rnd);
#endif

rnd = disconnSynAck8->rnd;

NetSend(DISCONN_ACK_N);
phase = 5;
phaseTime = GetTickCount();

return DISCONN_SYN_ACK_N;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Process_FIN_8(const char *)
{
phase = 1;
phaseTime = GetTickCount();
if (pStatusChangedCb != NULL)
    pStatusChangedCb(0, statusChangedCbData);

return FIN_N;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Process_INFO_8(const char * buffer)
{
info = (INFO_8*)buffer;

#ifdef ARCH_BE
SwapBytes(info->len);
SwapBytes(info->sendTime);
#endif

if (pInfoCb != NULL)
    pInfoCb((char*)info->text, info->infoType, info->showTime, info->sendTime, infoCbData);
return INFO_8_N;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Process_ERROR(const char * buffer)
{
ERR_8 err;
memcpy(&err, buffer, sizeof(err));

#ifdef ARCH_BE
SwapBytes(err.len);
#endif

KOIToWin((const char*)err.text, &messageText);
if (pErrorCb != NULL)
    pErrorCb(messageText, IA_SERVER_ERROR, errorCbData);
phase = 1;
phaseTime = GetTickCount();
codeError = IA_SERVER_ERROR;

return ERROR_N;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Prepare_CONN_SYN_8(char * buffer)
{
connSyn8 = (CONN_SYN_8*)buffer;

#ifdef ARCH_BE
SwapBytes(connSyn8->len);
#endif

assert(sizeof(CONN_SYN_8) == Min8(sizeof(CONN_SYN_8)) && "CONN_SYN_8 is not aligned to 8 bytes");

connSyn8->len = sizeof(CONN_SYN_8);
strncpy((char*)connSyn8->type, "CONN_SYN", IA_MAX_TYPE_LEN);
strncpy((char*)connSyn8->login, login.c_str(), IA_LOGIN_LEN);
connSyn8->dirs = 0;
for (int i = 0; i < DIR_NUM; i++)
    {
    connSyn8->dirs |= (selectedDirs[i] << i);
    }
return connSyn8->len;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Prepare_CONN_ACK_8(char * buffer)
{
connAck8 = (CONN_ACK_8*)buffer;

#ifdef ARCH_BE
SwapBytes(connAck8->len);
SwapBytes(connAck8->rnd);
#endif

assert(sizeof(CONN_ACK_8) == Min8(sizeof(CONN_ACK_8)) && "CONN_ACK_8 is not aligned to 8 bytes");

connAck8->len = sizeof(CONN_ACK_8);
strncpy((char*)connAck8->loginS, login.c_str(), IA_LOGIN_LEN);
strncpy((char*)connAck8->type, "CONN_ACK", IA_MAX_TYPE_LEN);
rnd++;
connAck8->rnd = rnd;

return connAck8->len;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Prepare_ALIVE_ACK_8(char * buffer)
{
aliveAck8 = (ALIVE_ACK_8*)buffer;

#ifdef ARCH_BE
SwapBytes(aliveAck8->len);
SwapBytes(aliveAck8->rnd);
#endif

assert(Min8(sizeof(ALIVE_ACK_8)) == sizeof(ALIVE_ACK_8) && "ALIVE_ACK_8 is not aligned to 8 bytes");

aliveAck8 = (ALIVE_ACK_8*)buffer;
aliveAck8->len = sizeof(ALIVE_ACK_8);
strncpy((char*)aliveAck8->loginS, login.c_str(), IA_LOGIN_LEN);
strncpy((char*)aliveAck8->type, "ALIVE_ACK", IA_MAX_TYPE_LEN);
aliveAck8->rnd = ++rnd;
return aliveAck8->len;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Prepare_DISCONN_SYN_8(char * buffer)
{
disconnSyn8 = (DISCONN_SYN_8*)buffer;

#ifdef ARCH_BE
SwapBytes(disconnSyn8->len);
#endif

assert(Min8(sizeof(DISCONN_SYN_8)) == sizeof(DISCONN_SYN_8) && "DISCONN_SYN_8 is not aligned to 8 bytes");

disconnSyn8->len = sizeof(DISCONN_SYN_8);
strncpy((char*)disconnSyn8->loginS, login.c_str(), IA_LOGIN_LEN);
strncpy((char*)disconnSyn8->type, "DISCONN_SYN", IA_MAX_TYPE_LEN);
strncpy((char*)disconnSyn8->login, login.c_str(), IA_LOGIN_LEN);
return disconnSyn8->len;
}
//---------------------------------------------------------------------------
int IA_CLIENT_PROT::Prepare_DISCONN_ACK_8(char * buffer)
{
disconnAck8 = (DISCONN_ACK_8*)buffer;

#ifdef ARCH_BE
SwapBytes(disconnAck8->len);
SwapBytes(disconnAck8->rnd);
#endif

assert(Min8(sizeof(DISCONN_ACK_8)) == sizeof(DISCONN_ACK_8) && "DISCONN_ACK_8 is not aligned to 8 bytes");

disconnAck8->len = Min8(sizeof(DISCONN_ACK_8));
disconnAck8->rnd = rnd + 1;
strncpy((char*)disconnAck8->loginS, login.c_str(), IA_LOGIN_LEN);
strncpy((char*)disconnAck8->type, "DISCONN_ACK", IA_MAX_TYPE_LEN);
return disconnAck8->len;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetStatusChangedCb(tpStatusChangedCb p, void * data)
{
pStatusChangedCb = p;
statusChangedCbData = data;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetStatChangedCb(tpStatChangedCb p, void * data)
{
pStatChangedCb = p;
statChangedCbData = data;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetInfoCb(tpCallBackInfoFn p, void * data)
{
pInfoCb = p;
infoCbData = data;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetDirNameCb(tpCallBackDirNameFn p, void * data)
{
pDirNameCb = p;
dirNameCbData = data;
}
//---------------------------------------------------------------------------
void IA_CLIENT_PROT::SetErrorCb(tpCallBackErrorFn p, void * data)
{
pErrorCb = p;
errorCbData = data;
}
//---------------------------------------------------------------------------
