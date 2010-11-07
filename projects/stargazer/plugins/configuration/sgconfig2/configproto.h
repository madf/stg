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
 $Revision: 1.12 $
 $Date: 2009/10/15 14:46:17 $
 $Author: faust $
 */


#ifndef CONFIGPROTO_H
#define CONFIGPROTO_H

#include <expat.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <string>

#include "parser.h"
#include "../../../users.h"
#include "../../../admins.h"
#include "../../../tariffs.h"
#include "stg_logger.h"

using namespace std;

#define  STG_HEADER     "SG04"
#define  OK_HEADER      "OKHD"
#define  ERR_HEADER     "ERHD"
#define  OK_LOGIN       "OKLG"
#define  ERR_LOGIN      "ERLG"
#define  OK_LOGINS      "OKLS"
#define  ERR_LOGINS     "ERLS"

//-----------------------------------------------------------------------------
class CONFIGPROTO
{
public:
    CONFIGPROTO();
    ~CONFIGPROTO();

    void            SetPort(uint16_t port);
    //void            SetHostAllow(HOSTALLOW * ha);
    void            SetAdmins(ADMINS * a);
    void            SetUsers(USERS * u);
    void            SetTariffs(TARIFFS * t);
    void            SetStore(BASE_STORE * s);
    void            SetStgSettings(const SETTINGS * s);
    const string &  GetAdminLogin();
    uint32_t        GetAdminIP();
    int             Prepare();
    int             Stop();
    const string &  GetStrError();
    static void *   Run(void * a);

private:
    int             RecvHdr(int sock);
    int             RecvLogin(int sock);
    int             SendLoginAnswer(int sock, int err);
    int             SendHdrAnswer(int sock, int err);
    int             RecvLoginS(int sock);
    int             SendLoginSAnswer(int sock, int err);
    int             RecvData(int sock);
    int             SendDataAnswer(int sock);
    void            SendError(const char * text);
    void            WriteLogAccessFailed(uint32_t ip);

    int             ParseCommand();

    list<string>    answerList;
    list<string>    requestList;
    uint32_t        adminIP;
    string          adminLogin;
    uint16_t        port;
    pthread_t       thrReciveSendConf;
    bool            nonstop;
    int             state;
    //HOSTALLOW *     hostAllow;
    const ADMIN *   currAdmin;
    STG_LOGGER &    WriteServLog;

    int                 outerSocket;
    int                 listenSocket;
    struct sockaddr_in  outerAddr;
    socklen_t           outerAddrLen;

    PARSER_GET_SERVER_INFO      parserGetServInfo;

    PARSER_GET_USERS            parserGetUsers;
    PARSER_GET_USER             parserGetUser;
    PARSER_CHG_USER             parserChgUser;
    PARSER_ADD_USER             parserAddUser;
    PARSER_DEL_USER             parserDelUser;
    PARSER_CHECK_USER           parserCheckUser;
    PARSER_SEND_MESSAGE         parserSendMessage;

    PARSER_GET_ADMINS           parserGetAdmins;
    PARSER_ADD_ADMIN            parserAddAdmin;
    PARSER_DEL_ADMIN            parserDelAdmin;
    PARSER_CHG_ADMIN            parserChgAdmin;

    PARSER_GET_TARIFFS          parserGetTariffs;
    PARSER_ADD_TARIFF           parserAddTariff;
    PARSER_DEL_TARIFF           parserDelTariff;
    PARSER_CHG_TARIFF           parserChgTariff;

    ADMINS *                    admins;
    USERS *                     users;
    TARIFFS *                   tariffs;
    BASE_STORE *                store;
    const SETTINGS *            settings;

    BASE_PARSER *               currParser;
    vector<BASE_PARSER*>        dataParser;

    XML_Parser                  xmlParser;

    string                      errorStr;

    friend void ParseXMLStart(void *data, const char *el, const char **attr);
    friend void ParseXMLEnd(void *data, const char *el);
};
//-----------------------------------------------------------------------------
#endif //CONFIGPROTO_H

