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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#include "parser_server_info.h"

#include "stg/settings.h"
#include "stg/users.h"
#include "stg/tariffs.h"
#include "stg/version.h"
#include "stg/const.h"

#include <string>
#include <cstring>

#include <sys/utsname.h>

using STG::PARSER::GET_SERVER_INFO;

const char * GET_SERVER_INFO::tag = "GetServerInfo";

void GET_SERVER_INFO::CreateAnswer()
{
    struct utsname utsn;
    uname(&utsn);

    std::string name = std::string(utsn.sysname) + " " +
                       utsn.release + " " +
                       utsn.machine + " " +
                       utsn.nodename;

    m_answer = std::string("<ServerInfo><version value=\"") + SERVER_VERSION + "\"/>" +
               "<tariff_num value=\"" + std::to_string(m_tariffs.Count()) + "\"/>" +
               "<tariff value=\"2\"/>" +
               "<user_num value=\"" + std::to_string(m_users.Count()) + "\"/>" +
               "<uname value=\"" + name + "\"/>" +
               "<dir_num value=\"" + std::to_string(DIR_NUM) + "\"/>" +
               "<day_fee value=\"" + std::to_string(m_settings.GetDayFee()) + "\"/>";

    for (size_t i = 0; i< DIR_NUM; i++)
        m_answer += "<dir_name_" + std::to_string(i) + " value=\"" + Encode12str(m_settings.GetDirName(i)) + "\"/>";

    m_answer += "</ServerInfo>";
}
