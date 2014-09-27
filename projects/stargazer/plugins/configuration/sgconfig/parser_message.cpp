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

#include "parser_message.h"

#include "stg/users.h"

extern volatile time_t stgTime; // So sad...

using STG::PARSER::SEND_MESSAGE;

const char * SEND_MESSAGE::tag = "Message";

int SEND_MESSAGE::Start(void *, const char *el, const char **attr)
{
    if (strcasecmp(el, m_tag.c_str()) != 0)
        return -1;

    for (size_t i = 0; i < 14; i++)
        if (attr[i] == NULL)
        {
            m_result = res_params_error;
            CreateAnswer();
            printfd(__FILE__, "To few parameters\n");
            return 0;
        }

    for (size_t i = 0; i < 14; i += 2)
    {
        if (strcasecmp(attr[i], "login") == 0)
            ParseLogins(attr[i + 1]);

        if (strcasecmp(attr[i], "MsgVer") == 0)
        {
            str2x(attr[i + 1], m_msg.header.ver);
            if (m_msg.header.ver != 1)
                m_result = res_params_error;
        }

        if (strcasecmp(attr[i], "MsgType") == 0)
        {
            str2x(attr[i + 1], m_msg.header.type);
            if (m_msg.header.type != 1)
                m_result = res_params_error;
        }

        if (strcasecmp(attr[i], "Repeat") == 0)
        {
            str2x(attr[i + 1], m_msg.header.repeat);
            if (m_msg.header.repeat < 0)
                m_result = res_params_error;
        }

        if (strcasecmp(attr[i], "RepeatPeriod") == 0)
            str2x(attr[i + 1], m_msg.header.repeatPeriod);

        if (strcasecmp(attr[i], "ShowTime") == 0)
            str2x(attr[i + 1], m_msg.header.showTime);

        if (strcasecmp(attr[i], "Text") == 0)
        {
            Decode21str(m_msg.text, attr[i + 1]);
            m_result = res_ok;
        }
    }
    return 0;
}

int SEND_MESSAGE::End(void *, const char *el)
{
    if (strcasecmp(el, m_tag.c_str()) != 0)
        return -1;

    m_result = res_unknown;
    for (unsigned i = 0; i < m_logins.size(); i++)
    {
        if (m_users.FindByName(m_logins[i], &m_user))
        {
            printfd(__FILE__, "User not found. %s\n", m_logins[i].c_str());
            continue;
        }
        m_msg.header.creationTime = static_cast<unsigned int>(stgTime);
        m_user->AddMessage(&m_msg);
        m_result = res_ok;
    }
    CreateAnswer();
    m_done = true;
    return 0;
}

int SEND_MESSAGE::ParseLogins(const char * login)
{
    char * p;
    char * l = new char[strlen(login) + 1];
    strcpy(l, login);
    p = strtok(l, ":");
    m_logins.clear();
    while (p)
    {
        m_logins.push_back(p);
        p = strtok(NULL, ":");
    }

    delete[] l;
    return 0;
}

void SEND_MESSAGE::CreateAnswer()
{
    switch (m_result)
    {
        case res_ok:
            m_answer = "<SendMessageResult value=\"ok\"/>";
            break;
        case res_params_error:
            m_answer = "<SendMessageResult value=\"Parameters error.\"/>";
            break;
        case res_unknown:
            m_answer = "<SendMessageResult value=\"Unknown user.\"/>";
            break;
    }
}
