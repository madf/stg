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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#include "parser_user_info.h"

#include "stg/user.h"
#include "stg/common.h"

#include <strings.h> // strcasecmp

int PARSER_USER_INFO::ParseStart(void * /*data*/, const char *el, const char **attr)
{
login.clear();
if (strcasecmp(el, "GetUserInfo") != 0)
    return -1;

if (!attr[0] || !attr[1] || strcasecmp(attr[0], "login") != 0)
    return -1;

login = attr[1];
return 0;
}

int PARSER_USER_INFO::ParseEnd(void * /*data*/, const char *el)
{
if (strcasecmp(el, "GetUserInfo") != 0)
    return -1;

CreateAnswer();
return 0;
}

void PARSER_USER_INFO::CreateAnswer()
{
CONST_USER_PTR u;
if (users->FindByName(login, &u))
    {
    answer = "<UserInfo result=\"error\"/>";
    return;
    }

answer = "<UserInfo lastAuthTime=\"" + x2str(u->GetAuthorizedModificationTime()) + "\"" +
         " lastDisconnectTime=\"" + x2str(u->GetConnectedModificationTime()) + "\"" +
         " connected=\"" + (u->GetConnected() ? "true" : "false") + "\"" +
         " lastDisconnectReason=\"" + u->GetLastDisconnectReason() + "\">";
std::vector<std::string> list(u->GetAuthorizers());
for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it)
    answer += "<Auth name=\"" + *it + "\"/>";
answer += "</UserInfo>";
}
