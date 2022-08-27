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
#pragma once

#include "stg/blowfish.h"
#include "stg/ia_packets.h"

#include <string>
#include <vector>
#include <map>

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#else
#include <winsock2.h>
#endif

#define IA_BIND_ERROR           (1)
#define IA_SERVER_ERROR         (2)
#define IA_FCNTL_ERROR          (3)
#define IA_GETHOSTBYNAME_ERROR  (4)

#define IA_PROTO_VER            (8)
#define IA_PROTO_PROXY_VER      (101)

typedef void (*tpStatusChangedCb)(int status, void * data);
typedef void (*tpStatChangedCb)(const LOADSTAT & stat, void * data);
typedef void (*tpCallBackInfoFn)(const std::string & message, int infoType, int showTime, int sendTime, void * data);
typedef void (*tpCallBackErrorFn)(const std::string & message, int netError, void * data);
typedef void (*tpCallBackDirNameFn)(const std::vector<std::string> & dirName, void * data);

//---------------------------------------------------------------------------
class IA_CLIENT_PROT
{
#ifdef WIN32
friend unsigned long WINAPI RunW(void * data);
#else
friend void * RunL(void * data);
#endif

    public:
        IA_CLIENT_PROT(const std::string & sn, uint16_t p, const std::string & localName = "", uint16_t localPort = 0);
        ~IA_CLIENT_PROT();

        void        Start();
        void        Stop();
        void        GetStat(LOADSTAT * ls);

        void        SetServer(const std::string & sn, unsigned short port);
        void        SetLogin(const std::string & login);
        void        SetPassword(const std::string & password);
        void        SetEnabledDirs(const bool * selectedDirs);

        void        SetStatusChangedCb(tpStatusChangedCb p, void * data);
        void        SetStatChangedCb(tpStatChangedCb p, void * data);
        void        SetInfoCb(tpCallBackInfoFn p, void * data);
        void        SetErrorCb(tpCallBackErrorFn p, void * data);
        void        SetDirNameCb(tpCallBackDirNameFn p, void * data);

        int         Connect();
        int         Disconnect();
        int         GetAuthorized() const { return m_phase == 3 || m_phase == 4; };
        int         GetPhase() const { return m_phase; };
        int         GetStatus() const;
        int         GetReconnect() const { return m_reconnect; };
        void        SetReconnect(int r) { m_reconnect = r; };
        char        GetProtoVer() const { return m_proxyMode ? IA_PROTO_PROXY_VER : IA_PROTO_VER; };
        void        GetMessageText(std::string * text) const { *text = m_messageText; };
        void        GetInfoText(std::string * text) const { *text = m_infoText; };
        int         GetStrError(std::string * error) const;

        void        SetProxyMode(bool on) { m_proxyMode = on; };
        bool        GetProxyMode() const { return m_proxyMode; };

        void        SetIP(uint32_t ip) { m_ip = ip; };
        uint32_t    GetIP() const { return m_ip; };

    private:
        void            Run();
        int             NetRecv();
        int             NetSend(int n);
        bool            GetNonstop() const { return m_nonstop; };
        void            PrepareNet();
        int             DeterminatePacketType(const char * buffer);

        int             Process_CONN_SYN_ACK_8(const void* buffer);
        int             Process_ALIVE_SYN_8(const void* buffer);
        int             Process_DISCONN_SYN_ACK_8(const void* buffer);
        int             Process_FIN_8(const void* buffer);
        int             Process_INFO_8(const void* buffer);
        int             Process_ERROR(const void* buffer);

        int             Prepare_CONN_SYN_8(void* buffer);
        int             Prepare_CONN_ACK_8(void* buffer);
        int             Prepare_ALIVE_ACK_8(void* buffer);
        int             Prepare_DISCONN_SYN_8(void* buffer);
        int             Prepare_DISCONN_ACK_8(void* buffer);

        void            FillHdr8(char* buffer, unsigned long ip);
        int             Send(char * buffer, int len);
        int             Recv(char * buffer, int len);

        LOADSTAT        m_stat;
        int             m_action;
        int             m_phase;
        int             m_phaseTime;
        std::string     m_messageText;
        std::string     m_infoText;
        mutable std::string m_strError;
        mutable int     m_codeError;
        bool            m_nonstop;
        bool            m_isNetPrepared;
        bool            m_proxyMode;

        BLOWFISH_CTX    m_ctxPass;
        BLOWFISH_CTX    m_ctxHdr;

        bool            m_selectedDirs[DIR_NUM];

        std::string     m_password;
        std::string     m_login;

        #ifdef WIN32
        WSADATA m_wsaData;
        #else
        pthread_t m_thread;
        #endif

        std::string     m_serverName;
        uint16_t        m_port;
        uint32_t        m_ip;
        std::string     m_localName;
        uint32_t        m_localIP;
        uint32_t        m_localPort;

        struct sockaddr_in  m_servAddr;

        bool            m_firstConnect;
        int             m_reconnect;
        int             m_sockr;
        int             m_protNum;
        int             m_userTimeout;
        int             m_aliveTimeout;
        unsigned int    m_rnd;

        tpStatusChangedCb   m_pStatusChangedCb;
        tpStatChangedCb     m_pStatChangedCb;
        tpCallBackInfoFn    m_pInfoCb;
        tpCallBackErrorFn   m_pErrorCb;
        tpCallBackDirNameFn m_pDirNameCb;

        void              * m_statusChangedCbData;
        void              * m_statChangedCbData;
        void              * m_infoCbData;
        void              * m_errorCbData;
        void              * m_dirNameCbData;

        std::map<std::string, int> m_packetTypes;

        CONN_SYN_8        * m_connSyn8;
        const CONN_SYN_ACK_8    * m_connSynAck8;
        CONN_ACK_8        * m_connAck8;
        const ALIVE_SYN_8       * m_aliveSyn8;
        ALIVE_ACK_8       * m_aliveAck8;
        DISCONN_SYN_8     * m_disconnSyn8;
        const DISCONN_SYN_ACK_8 * m_disconnSynAck8;
        DISCONN_ACK_8     * m_disconnAck8;
        const INFO_8            * m_info;
};
//---------------------------------------------------------------------------
#ifdef WIN32
unsigned long WINAPI RunW(void *);
#else
void * RunW(void *);
#endif
