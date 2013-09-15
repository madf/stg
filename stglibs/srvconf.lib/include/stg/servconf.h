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

#include "stg/parser_auth_by.h"
#include "stg/parser_server_info.h"
#include "stg/parser_check_user.h"
#include "stg/parser_get_user.h"
#include "stg/parser_get_users.h"
#include "stg/parser_chg_user.h"
#include "stg/parser_send_message.h"

#include "stg/os_int.h"

#include <string>

class SERVCONF
{
public:
    SERVCONF(const std::string & server, uint16_t port,
             const std::string & login, const std::string & password);
    ~SERVCONF();

    int GetUsers(PARSER_GET_USERS::CALLBACK f, void * data);
    int GetUser(const std::string & login, PARSER_GET_USER::CALLBACK f, void * data);
    int ChgUser(const std::string & request, PARSER_CHG_USER::CALLBACK f, void * data);
    int AuthBy(const std::string & login, PARSER_AUTH_BY::CALLBACK f, void * data);
    int SendMessage(const std::string & request, PARSER_SEND_MESSAGE::CALLBACK f, void * data);
    int ServerInfo(PARSER_SERVER_INFO::CALLBACK f, void * data);
    int CheckUser(const std::string & login, const std::string & password, PARSER_CHECK_USER::CALLBACK f, void * data);

    const std::string & GetStrError() const;

private:
    class IMPL;
    IMPL * pImpl;
};
//-----------------------------------------------------------------------------

#endif  /* _SERVCONF_H_ */
