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
#include "stg/optional.h"

#include <string>

namespace STG
{

struct Admins;
struct Admin;

namespace PARSER
{

class GET_ADMINS: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(const Admins & admins) : m_admins(admins) {}
                BASE_PARSER * create(const Admin & admin) override { return new GET_ADMINS(admin, m_admins); }
                static void Register(REGISTRY & registry, const Admins & admins)
                { registry[ToLower(tag)] = new FACTORY(admins); }
            private:
                const Admins & m_admins;
        };

        static const char * tag;

        GET_ADMINS(const Admin & admin, const Admins & admins)
            : BASE_PARSER(admin, tag), m_admins(admins) {}

    private:
        const Admins & m_admins;

        void CreateAnswer() override;
};

class ADD_ADMIN: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(Admins & admins) : m_admins(admins) {}
                BASE_PARSER * create(const Admin & admin) override { return new ADD_ADMIN(admin, m_admins); }
                static void Register(REGISTRY & registry, Admins & admins)
                { registry[ToLower(tag)] = new FACTORY(admins); }
            private:
                Admins & m_admins;
        };

        static const char * tag;

        ADD_ADMIN(const Admin & admin, Admins & admins)
            : BASE_PARSER(admin, tag), m_admins(admins) {}
        int Start(void * data, const char * el, const char ** attr) override;

    private:
        std::string m_admin;
        Admins & m_admins;

        void CreateAnswer() override;
};

class DEL_ADMIN: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(Admins & admins) : m_admins(admins) {}
                BASE_PARSER * create(const Admin & admin) override { return new DEL_ADMIN(admin, m_admins); }
                static void Register(REGISTRY & registry, Admins & admins)
                { registry[ToLower(tag)] = new FACTORY(admins); }
            private:
                Admins & m_admins;
        };

        static const char * tag;

        DEL_ADMIN(const Admin & admin, Admins & admins)
            : BASE_PARSER(admin, tag), m_admins(admins) {}
        int Start(void * data, const char * el, const char ** attr) override;

    private:
        std::string m_admin;
        Admins & m_admins;

        void CreateAnswer() override;
};

class CHG_ADMIN: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(Admins & admins) : m_admins(admins) {}
                BASE_PARSER * create(const Admin & admin) override { return new CHG_ADMIN(admin, m_admins); }
                static void Register(REGISTRY & registry, Admins & admins)
                { registry[ToLower(tag)] = new FACTORY(admins); }
            private:
                Admins & m_admins;
        };

        static const char * tag;

        CHG_ADMIN(const Admin & admin, Admins & admins)
            : BASE_PARSER(admin, tag), m_admins(admins) {}
        int Start(void * data, const char * el, const char ** attr) override;

    private:
        std::string login;
        Optional<std::string> password;
        Optional<std::string> privAsString;
        Admins & m_admins;

        void CreateAnswer() override;
};

} // namespace PARSER
} // namespace STG
