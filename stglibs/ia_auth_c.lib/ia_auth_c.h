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
 $Revision: 1.10 $
 $Date: 2010/03/15 12:57:24 $
*/

/*
* Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
*/
//---------------------------------------------------------------------------
#ifndef IA_AUTH_C_H
#define IA_AUTH_C_H

#ifndef WIN32
#include <netinet/in.h>
#include <pthread.h>
#else
#include <winsock2.h>
#endif

#include <string>
#include <vector>
#include <map>

#include "blowfish.h"
#include "ia_packets.h"

#define IA_BIND_ERROR           (1)
#define IA_SERVER_ERROR         (2)
#define IA_FCNTL_ERROR          (3)
#define IA_GETHOSTBYNAME_ERROR  (4)

#define IA_PROTO_VER            (8)
#define IA_PROTO_PROXY_VER      (101)

using namespace std;

typedef void (*tpStatusChangedCb)(int status, void * data);
typedef void (*tpStatChangedCb)(const LOADSTAT & stat, void * data);
typedef void (*tpCallBackInfoFn)(const string & message, int infoType, int showTime, int sendTime, void * data);
typedef void (*tpCallBackErrorFn)(const string & message, int netError, void * data);
typedef void (*tpCallBackDirNameFn)(const vector<string> & dirName, void * data);

//---------------------------------------------------------------------------
class IA_CLIENT_PROT
{
#ifdef WIN32
friend unsigned long WINAPI RunW(void * data);
#else
friend void * RunL(void * data);
#endif

public:
    IA_CLIENT_PROT(const string & sn, uint16_t p, uint16_t localPort = 0);
    ~IA_CLIENT_PROT();

    void        Start();
    void        Stop();
    void        GetStat(LOADSTAT * ls);

    void        SetServer(const string & sn, unsigned short port);
    void        SetLogin(const string & login);
    void        SetPassword(const string & password);
    void        SetEnabledDirs(const bool * selectedDirs);

    void        SetStatusChangedCb(tpStatusChangedCb p, void * data);
    void        SetStatChangedCb(tpStatChangedCb p, void * data);
    void        SetInfoCb(tpCallBackInfoFn p, void * data);
    void        SetErrorCb(tpCallBackErrorFn p, void * data);
    void        SetDirNameCb(tpCallBackDirNameFn p, void * data);

    int         Connect();
    int         Disconnect();
    int         GetAuthorized() const { return phase == 3 || phase == 4; };
    int         GetPhase() const { return phase; };
    int         GetStatus() const;
    int         GetReconnect() const { return reconnect; };
    void        SetReconnect(int r) { reconnect = r; };
    char        GetProtoVer() const { return proxyMode ? IA_PROTO_PROXY_VER : IA_PROTO_VER; };
    void        GetMessageText(string * text) const { *text = messageText; };
    void        GetInfoText(string * text) const { *text = infoText; };
    int         GetStrError(string * error) const;

    void        SetProxyMode(bool on) { proxyMode = on; };
    bool        GetProxyMode() const { return proxyMode; };

    void        SetIP(uint32_t ip) { IA_CLIENT_PROT::ip = ip; };
    uint32_t    GetIP() const { return ip; };

private:
    void            Run();
    int             NetRecv();
    int             NetSend(int n);
    bool            GetNonstop() const { return nonstop; };
    void            PrepareNet();
    int             DeterminatePacketType(const char * buffer);

    int             Process_CONN_SYN_ACK_8(const char * buffer);
    int             Process_ALIVE_SYN_8(const char * buffer);
    int             Process_DISCONN_SYN_ACK_8(const char * buffer);
    int             Process_FIN_8(const char * buffer);
    int             Process_INFO_8(const char * buffer);
    int             Process_ERROR(const char * buffer);

    int             Prepare_CONN_SYN_8(char * buffer);
    int             Prepare_CONN_ACK_8(char * buffer);
    int             Prepare_ALIVE_ACK_8(char * buffer);
    int             Prepare_DISCONN_SYN_8(char * buffer);
    int             Prepare_DISCONN_ACK_8(char * buffer);

    void            FillHdr8(char * buffer, unsigned long ip);
    int             Send(char * buffer, int len);
    int             Recv(char * buffer, int len);

    LOADSTAT        stat;
    int             action;
    int             phase;
    int             phaseTime;
    string          messageText;
    string          infoText;
    mutable string  strError;
    mutable int     codeError;
    bool            nonstop;
    bool            isNetPrepared;
    bool            proxyMode;

    BLOWFISH_CTX    ctxPass;
    BLOWFISH_CTX    ctxHdr;

    bool            selectedDirs[DIR_NUM];

    string          password;
    string          login;

    #ifdef WIN32
    WSADATA wsaData;
    #else
    pthread_t thread;
    #endif

    string          serverName;
    uint16_t        port;
    uint32_t        ip;
    uint32_t        localPort;

    struct sockaddr_in  servAddr;

    bool            firstConnect;
    int             reconnect;
    int             sockr;
    int             protNum;
    int             userTimeout;
    int             aliveTimeout;
    unsigned int    rnd;

    tpStatusChangedCb   pStatusChangedCb;
    tpStatChangedCb     pStatChangedCb;
    tpCallBackInfoFn    pInfoCb;
    tpCallBackErrorFn   pErrorCb;
    tpCallBackDirNameFn pDirNameCb;

    void              * statusChangedCbData;
    void              * statChangedCbData;
    void              * infoCbData;
    void              * errorCbData;
    void              * dirNameCbData;

    map<string, int>    packetTypes;

    CONN_SYN_8        * connSyn8;
    CONN_SYN_ACK_8    * connSynAck8;
    CONN_ACK_8        * connAck8;
    ALIVE_SYN_8       * aliveSyn8;
    ALIVE_ACK_8       * aliveAck8;
    DISCONN_SYN_8     * disconnSyn8;
    DISCONN_SYN_ACK_8 * disconnSynAck8;
    DISCONN_ACK_8     * disconnAck8;
    INFO_8            * info;
};
//---------------------------------------------------------------------------
#ifdef WIN32
unsigned long WINAPI RunW(void *);
#else
void * RunW(void *);
#endif

//---------------------------------------------------------------------------
#endif //IA_AUTH_C_H
