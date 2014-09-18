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
 $Revision: 1.14 $
 $Date: 2010/10/04 20:24:14 $
 $Author: faust $
 */


#ifndef CONFIGPROTO_H
#define CONFIGPROTO_H

#include <string>
#include <list>
#include <vector>

#include "stg/module_settings.h"
#include "stg/os_int.h"

#include <expat.h>
#include <pthread.h>

#define  STG_HEADER     "SG04"
#define  OK_HEADER      "OKHD"
#define  ERR_HEADER     "ERHD"
#define  OK_LOGIN       "OKLG"
#define  ERR_LOGIN      "ERLG"
#define  OK_LOGINS      "OKLS"
#define  ERR_LOGINS     "ERLS"

class BASE_PARSER;
class USERS;
class ADMINS;
class ADMIN;
class TARIFFS;
class PLUGIN_LOGGER;
class STORE;
class SETTINGS;

//-----------------------------------------------------------------------------
class CONFIGPROTO {
public:
    CONFIGPROTO(PLUGIN_LOGGER & l);
    ~CONFIGPROTO();

    void            SetPort(uint16_t port);
    void            SetAdmins(ADMINS * a);
    uint32_t        GetAdminIP() const { return adminIP; }
    int             Prepare();
    int             Stop();
    const std::string & GetStrError() const { return errorStr; }
    void            Run();

private:
    CONFIGPROTO(const CONFIGPROTO & rvalue);
    CONFIGPROTO & operator=(const CONFIGPROTO & rvalue);

    int             RecvHdr(int sock);
    int             RecvLogin(int sock);
    int             SendLoginAnswer(int sock);
    int             SendHdrAnswer(int sock, int err);
    int             RecvLoginS(int sock);
    int             SendLoginSAnswer(int sock, int err);
    int             RecvData(int sock);
    int             SendDataAnswer(int sock, const std::string & answer);
    int             SendError(int sock, const std::string & text);
    void            WriteLogAccessFailed(uint32_t ip);
    const std::string & GetDataAnswer() const { return dataAnswer; }

    int             ParseCommand();

    std::list<std::string>      requestList;
    uint32_t                    adminIP;
    std::string                 adminLogin;
    std::string                 adminPassword;
    uint16_t                    port;
    pthread_t                   thrReciveSendConf;
    bool                        nonstop;
    int                         state;
    ADMIN *                     currAdmin;
    PLUGIN_LOGGER &             logger;
    std::string                 dataAnswer;

    int                         listenSocket;

    ADMINS *                    admins;

    BASE_PARSER *               currParser;
    std::vector<BASE_PARSER *>  dataParser;

    XML_Parser                  xmlParser;

    std::string                 errorStr;

    friend void ParseXMLStart(void *data, const char *el, const char **attr);
    friend void ParseXMLEnd(void *data, const char *el);
};
//-----------------------------------------------------------------------------
#endif //CONFIGPROTO_H
