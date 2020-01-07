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

#include "stg/users.h"
#include "stg/user.h"

#include <strings.h> // strcasecmp

using STG::PARSER::USER_INFO;

const char * USER_INFO::tag = "GetUserInfo";

int USER_INFO::Start(void * /*data*/, const char *el, const char **attr)
{
    if (strcasecmp(el, m_tag.c_str()) != 0)
        return -1;

    if (!attr[1])
        return -1;

    m_login = attr[1];
    return 0;
}

void USER_INFO::CreateAnswer()
{
    CONST_USER_PTR u;
    if (m_users.FindByName(m_login, &u))
    {
        m_answer = "<UserInfo result=\"error\"/>";
        return;
    }

    m_answer = "<UserInfo lastAuthTime=\"" + x2str(u->GetAuthorizedModificationTime()) + "\"" +
             " lastDisconnectTime=\"" + x2str(u->GetConnectedModificationTime()) + "\"" +
             " connected=\"" + (u->GetConnected() ? "true" : "false") + "\"" +
             " lastDisconnectReason=\"" + u->GetLastDisconnectReason() + "\">";
    std::vector<std::string> list(u->GetAuthorizers());
    for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it)
        m_answer += "<Auth name=\"" + *it + "\"/>";
    m_answer += "</UserInfo>";
}
