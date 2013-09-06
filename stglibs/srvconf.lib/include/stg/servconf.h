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
#include "stg/parser_get_user.h"
#include "stg/parser_get_users.h"
#include "stg/parser_chg_user.h"
#include "stg/parser_send_message.h"

#include "stg/os_int.h"
#include "stg/const.h"

#include <list>
#include <vector>
#include <string>

#include <expat.h>

#define MAX_ERR_STR_LEN (64)
#define IP_STRING_LEN   (255)

struct ADMINDATA
{
    char login[ADM_LOGIN_LEN];
};
//-----------------------------------------------------------------------------
class SERVCONF
{
public:
    SERVCONF(const std::string & server, uint16_t port,
             const std::string & login, const std::string & password);

    void SetGetUsersCallback(PARSER_GET_USERS::CALLBACK f, void * data);
    void SetAuthByCallback(PARSER_AUTH_BY::CALLBACK f, void * data);
    void SetServerInfoCallback(PARSER_SERVER_INFO::CALLBACK f, void * data);
    void SetChgUserCallback(PARSER_CHG_USER::CALLBACK f, void * data);
    void SetCheckUserCallback(PARSER_CHECK_USER::CALLBACK f, void * data);
    void SetGetUserCallback(PARSER_GET_USER::CALLBACK f, void * data);
    void SetSendMessageCallback(PARSER_SEND_MESSAGE::CALLBACK f, void * data);

    int GetUsers();
    int GetUser(const char * login);
    int ChgUser(const char * request);
    int AuthBy(const char * login);
    int SendMessage(const char * request);
    int ServerInfo();
    int CheckUser(const char * login, const char * password);

    const std::string & GetStrError() const;
    int Start(const char * el, const char ** attr);
    void End(const char * el);

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

    std::string errorMsg;
    XML_Parser parser;

    int Exec(const char * request);

    friend bool AnsRecv(void * data, const std::string & chunk, bool final);
};
//-----------------------------------------------------------------------------

#endif  /* _SERVCONF_H_ */
