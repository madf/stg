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

#include "stg/tariff_conf.h"
#include "stg/common.h"

#include <string>

namespace STG
{

struct Tariffs;
struct Users;
struct Admin;

namespace PARSER
{

class GET_TARIFFS: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(const Tariffs & tariffs) : m_tariffs(tariffs) {}
                virtual BASE_PARSER * create(const Admin & admin) { return new GET_TARIFFS(admin, m_tariffs); }
                static void Register(REGISTRY & registry, const Tariffs & tariffs)
                { registry[ToLower(tag)] = new FACTORY(tariffs); }
            private:
                const Tariffs & m_tariffs;
        };

        static const char * tag;

        GET_TARIFFS(const Admin & admin, const Tariffs & tariffs)
            : BASE_PARSER(admin, tag), m_tariffs(tariffs) {}

    private:
        const Tariffs & m_tariffs;

        void CreateAnswer();
};

class ADD_TARIFF: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(Tariffs & tariffs) : m_tariffs(tariffs) {}
                virtual BASE_PARSER * create(const Admin & admin) { return new ADD_TARIFF(admin, m_tariffs); }
                static void Register(REGISTRY & registry, Tariffs & tariffs)
                { registry[ToLower(tag)] = new FACTORY(tariffs); }
            private:
                Tariffs & m_tariffs;
        };

        static const char * tag;

        ADD_TARIFF(const Admin & admin, Tariffs & tariffs)
            : BASE_PARSER(admin, tag), m_tariffs(tariffs) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        std::string tariff;
        Tariffs & m_tariffs;

        void CreateAnswer();
};

class DEL_TARIFF: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                FACTORY(Tariffs & tariffs, const Users & users) : m_tariffs(tariffs), m_users(users) {}
                virtual BASE_PARSER * create(const Admin & admin) { return new DEL_TARIFF(admin, m_users, m_tariffs); }
                static void Register(REGISTRY & registry, Tariffs & tariffs, const Users & users)
                { registry[ToLower(tag)] = new FACTORY(tariffs, users); }
            private:
                Tariffs & m_tariffs;
                const Users & m_users;
        };

        static const char * tag;

        DEL_TARIFF(const Admin & admin, const Users & users, Tariffs & tariffs)
            : BASE_PARSER(admin, tag), m_users(users), m_tariffs(tariffs) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        std::string tariff;
        const Users & m_users;
        Tariffs & m_tariffs;

        void CreateAnswer();
};

class CHG_TARIFF: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(Tariffs & tariffs) : m_tariffs(tariffs) {}
                virtual BASE_PARSER * create(const Admin & admin) { return new CHG_TARIFF(admin, m_tariffs); }
                static void Register(REGISTRY & registry, Tariffs & tariffs)
                { registry[ToLower(tag)] = new FACTORY(tariffs); }
            private:
                Tariffs & m_tariffs;
        };

        static const char * tag;

        CHG_TARIFF(const Admin & admin, Tariffs & tariffs)
            : BASE_PARSER(admin, tag), m_tariffs(tariffs) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        TariffDataOpt td;
        Tariffs & m_tariffs;

        int CheckTariffData();
        void CreateAnswer();
};

} // namespace PARSER
} // namespace STG
