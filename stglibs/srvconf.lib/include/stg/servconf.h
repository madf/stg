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
 $Revision: 1.10 $
 $Date: 2009/03/17 09:52:35 $
 $Author: faust $
 */

#ifndef SERVCONF_H
#define SERVCONF_H

#include "netunit.h"

#include "stg/parser_auth_by.h"
#include "stg/parser_server_info.h"
#include "stg/parser_check_user.h"

#include "stg/os_int.h"
#include "stg/const.h"

#include <list>
#include <vector>
#include <string>

#include <expat.h>

void Start(void *data, const char *el, const char **attr);
void End(void *data, const char *el);

#define MAX_ERR_STR_LEN (64)
#define IP_STRING_LEN   (255)
//-----------------------------------------------------------------------------
typedef int(*RecvChgUserCb_t)(const char * asnwer, void * data);
typedef int(*RecvSendMessageCb_t)(const char * answer, void * data);
//-----------------------------------------------------------------------------
struct ADMINDATA
{
    char login[ADM_LOGIN_LEN];
};
//-----------------------------------------------------------------------------
class PARSER_CHG_USER: public PARSER
{
public:
    PARSER_CHG_USER();
    int  ParseStart(const char *el, const char **attr);
    void ParseEnd(const char *el);
    void ParseAnswer(const char *el, const char **attr);
    void SetChgUserRecvCb(RecvChgUserCb_t, void * data);
private:
    RecvChgUserCb_t RecvChgUserCb;
    void * chgUserCbData;
    int depth;
    bool error;
};
//-----------------------------------------------------------------------------
class PARSER_GET_USERS: public PARSER
{
public:
    PARSER_GET_USERS();
    int  ParseStart(const char *el, const char **attr);
    void ParseEnd(const char *el);
    void ParseUsers(const char *el, const char **attr);
    void ParseUser(const char *el, const char **attr);
    void ParseUserParams(const char *el, const char **attr);
    void ParseUserLoadStat(const char * el, const char ** attr);
    void SetUserDataRecvCb(RecvUserDataCb_t, void * data);
private:
    RecvUserDataCb_t RecvUserDataCb;
    void * userDataCb;
    USERDATA user;
    int depth;
    bool error;
};
//-----------------------------------------------------------------------------
class PARSER_SEND_MESSAGE: public PARSER
{
public:
    PARSER_SEND_MESSAGE();
    int  ParseStart(const char *el, const char **attr);
    void ParseEnd(const char *el);
    void ParseAnswer(const char *el, const char **attr);
    void SetSendMessageRecvCb(RecvSendMessageCb_t, void * data);
private:
    RecvSendMessageCb_t RecvSendMessageCb;
    void * sendMessageCbData;
    int depth;
    bool error;
};
//-----------------------------------------------------------------------------
class SERVCONF
{
public:
    SERVCONF();
    void SetServer(const char * server);
    void SetPort(uint16_t port);

    void SetAdmLogin(const char * login);
    void SetAdmPassword(const char * password);

    void SetUserDataRecvCb(RecvUserDataCb_t, void * data);
    void SetAuthByCallback(PARSER_AUTH_BY::CALLBACK f, void * data);
    void SetServerInfoCallback(PARSER_SERVER_INFO::CALLBACK f, void * data);
    void SetChgUserCb(RecvChgUserCb_t, void * data);
    void SetCheckUserCallback(PARSER_CHECK_USER::CALLBACK f, void * data);
    void SetGetUserCallback(PARSER_GET_USER::CALLBACK f, void * data);
    void SetSendMessageCb(RecvSendMessageCb_t, void * data);

    int GetUsers();
    int GetUser(const char * login);
    int ChgUser(const char * request);
    int AuthBy(const char * login);
    // TODO: Remove this shit!
    int MsgUser(const char * request);
    int SendMessage(const char * login, const char * message, int prio);
    int ServerInfo();
    int CheckUser(const char * login, const char * password);

    const std::string & GetStrError() const;
    int GetError();
    int Start(const char *el, const char **attr);
    void End(const char *el);

private:
    PARSER * currParser;

    PARSER_GET_USERS parserGetUsers;
    PARSER_GET_USER parserGetUser;
    PARSER_AUTH_BY parserAuthBy;
    PARSER_SERVER_INFO  parserServerInfo;
    PARSER_CHG_USER parserChgUser;
    PARSER_CHECK_USER parserCheckUser;
    PARSER_SEND_MESSAGE parserSendMessage;

    NETTRANSACT nt;
    int parseDepth;

    std::string errorMsg;
    int error;
    XML_Parser parser;

    RecvUserDataCb_t RecvUserDataCb;
    PARSER_GET_USER::CALLBACK getUserCallback;
    PARSER_AUTH_BY::CALLBACK authByCallback;
    PARSER_SERVER_INFO::CALLBACK serverInfoCallback;
    RecvChgUserCb_t RecvChgUserCb;
    PARSER_CHECK_USER::CALLBACK checkUserCallback;
    RecvSendMessageCb_t RecvSendMessageCb;

    void * getUserData;
    void * authByData;
    void * getUsersDataDataCb;
    void * serverInfoData;
    void * chgUserDataCb;
    void * checkUserData;
    void * sendMessageDataCb;

    friend int AnsRecv(void * data, std::list<std::string> * list);
};
//-----------------------------------------------------------------------------

#endif  /* _SERVCONF_H_ */
