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

#pragma once

#include "parser.h"

#include "stg/common.h"

#include <string>

namespace STG
{

class Users;

namespace PARSER
{

class USER_INFO : public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(const Users & users) : m_users(users) {}
                BASE_PARSER * create(const Admin & admin) override { return new USER_INFO(admin, m_users); }
                static void Register(REGISTRY & registry, const Users & users)
                { registry[ToLower(tag)] = new FACTORY(users); }
            private:
                const Users & m_users;
        };

        static const char * tag;

        USER_INFO(const Admin & admin, const Users & users)
            : BASE_PARSER(admin, tag), m_users(users) {}
        int Start(void * data, const char * el, const char ** attr) override;

    private:
        const Users & m_users;
        std::string m_login;

        void CreateAnswer() override;
};

} // namespace PARSER
} // namespace STG
