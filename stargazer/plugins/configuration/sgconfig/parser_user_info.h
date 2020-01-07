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

#ifndef __STG_PARSER_USER_INFO_H__
#define __STG_PARSER_USER_INFO_H__

#include "parser.h"

#include "stg/common.h"

#include <string>

class USERS;

namespace STG
{
namespace PARSER
{

class USER_INFO : public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                FACTORY(const USERS & users) : m_users(users) {}
                virtual BASE_PARSER * create(const ADMIN & admin) { return new USER_INFO(admin, m_users); }
                static void Register(REGISTRY & registry, const USERS & users)
                { registry[ToLower(tag)] = new FACTORY(users); }
            private:
                const USERS & m_users;
        };

        static const char * tag;

        USER_INFO(const ADMIN & admin, const USERS & users)
            : BASE_PARSER(admin, tag), m_users(users) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        const USERS & m_users;
        std::string m_login;

        void CreateAnswer();
};

} // namespace PARSER
} // namespace STG

#endif
