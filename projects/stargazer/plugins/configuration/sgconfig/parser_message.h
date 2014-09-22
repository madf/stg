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

#ifndef __STG_SGCONFIG_PARSER_SEND_MESSAGE_H__
#define __STG_SGCONFIG_PARSER_SEND_MESSAGE_H__

#include "parser.h"

#include "stg/message.h"

#include <vector>
#include <string>

class USERS;
class USER;

namespace STG
{
namespace PARSER
{

class SEND_MESSAGE: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                FACTORY(USERS & users) : m_users(users) {}
                virtual BASE_PARSER * create(const ADMIN & admin) { return new SEND_MESSAGE(admin, m_users); }
                static void Register(REGISTRY & registry, USERS & users)
                { registry[tag] = new FACTORY(users); }
            private:
                USERS & m_users;
        };

        static const char * tag;

        SEND_MESSAGE(const ADMIN & admin, USERS & users)
            : BASE_PARSER(admin, tag), m_users(users), m_result(res_ok), m_user(NULL) {}
        int Start(void *data, const char *el, const char **attr);
        int End(void *data, const char *el);

    private:
        USERS & m_users;
        std::vector<std::string> m_logins;
        enum { res_ok, res_params_error, res_unknown } m_result;
        STG_MSG m_msg;
        USER * m_user;

        int ParseLogins(const char * logins);
        void CreateAnswer();
};

} // namespace PARSER
} // namespace STG

#endif
