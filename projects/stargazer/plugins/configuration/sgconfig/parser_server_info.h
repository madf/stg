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

#pragma once

#include "parser.h"

#include "stg/common.h"

namespace STG
{

class Admin;
struct Settings;
class Users;
class Tariffs;

namespace PARSER
{

class GET_SERVER_INFO: public BASE_PARSER {
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                FACTORY(const Settings & settings, const Users & users, const Tariffs & tariffs)
                    : m_settings(settings), m_users(users), m_tariffs(tariffs) {}
                BASE_PARSER * create(const Admin & admin) override { return new GET_SERVER_INFO(admin, m_settings, m_users, m_tariffs); }
                static void Register(REGISTRY & registry, const Settings & settings, const Users & users, const Tariffs & tariffs)
                { registry[ToLower(tag)] = new FACTORY(settings, users, tariffs); }
            private:
                const Settings & m_settings;
                const Users & m_users;
                const Tariffs & m_tariffs;
        };

        static const char * tag;

        GET_SERVER_INFO(const Admin & admin,
                        const Settings & settings,
                        const Users & users,
                        const Tariffs & tariffs)
            : BASE_PARSER(admin, tag),
              m_settings(settings),
              m_users(users),
              m_tariffs(tariffs)
        {}

    private:
        const Settings & m_settings;
        const Users & m_users;
        const Tariffs & m_tariffs;

        void CreateAnswer() override;
};

}
}
