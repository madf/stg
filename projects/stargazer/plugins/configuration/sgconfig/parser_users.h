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

#include "stg/user_conf.h"
#include "stg/user_stat.h"
#include "stg/common.h"

#include <string>
#include <optional>

namespace STG
{

struct Users;
struct User;
struct Tariffs;
struct Admin;
struct Store;

namespace PARSER
{

class GET_USERS: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(Users & users) : m_users(users) {}
                BASE_PARSER * create(const Admin & admin) override { return new GET_USERS(admin, m_users); }
                static void Register(REGISTRY & registry, Users & users)
                { registry[ToLower(tag)] = new FACTORY(users); }
            private:
                Users & m_users;
        };

        static const char * tag;

        GET_USERS(const Admin & admin, Users & users)
            : BASE_PARSER(admin, tag), m_users(users),
              m_lastUserUpdateTime(0) {}
        int Start(void * data, const char * el, const char ** attr) override;

    private:
        Users & m_users;
        time_t m_lastUserUpdateTime;

        void CreateAnswer() override;
};

class GET_USER: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(const Users & users) : m_users(users) {}
                BASE_PARSER * create(const Admin & admin) override { return new GET_USER(admin, m_users); }
                static void Register(REGISTRY & registry, const Users & users)
                { registry[ToLower(tag)] = new FACTORY(users); }
            private:
                const Users & m_users;
        };

        static const char * tag;

        GET_USER(const Admin & admin, const Users & users)
            : BASE_PARSER(admin, tag), m_users(users) {}
        int Start(void * data, const char * el, const char ** attr) override;

    private:
        const Users & m_users;
        std::string m_login;

        void CreateAnswer() override;
};

class ADD_USER: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(Users & users) : m_users(users) {}
                BASE_PARSER * create(const Admin & admin) override { return new ADD_USER(admin, m_users); }
                static void Register(REGISTRY & registry, Users & users)
                { registry[ToLower(tag)] = new FACTORY(users); }
            private:
                Users & m_users;
        };

        static const char * tag;

        ADD_USER(const Admin & admin, Users & users)
            : BASE_PARSER(admin, tag), m_users(users) {}
        int Start(void * data, const char * el, const char ** attr) override;

    private:
        Users & m_users;
        std::string m_login;

        void CreateAnswer() override;
};

class CHG_USER: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                FACTORY(Users & users, Store & store, const Tariffs & tariffs)
                    : m_users(users), m_store(store), m_tariffs(tariffs)
                {}
                BASE_PARSER * create(const Admin & admin) override { return new CHG_USER(admin, m_users, m_store, m_tariffs); }
                static void Register(REGISTRY & registry, Users & users, Store & store, const Tariffs & tariffs)
                { registry[ToLower(tag)] = new FACTORY(users, store, tariffs); }
            private:
                Users & m_users;
                Store & m_store;
                const Tariffs & m_tariffs;
        };

        static const char * tag;

        CHG_USER(const Admin & admin, Users & users,
                 Store & store, const Tariffs & tariffs)
            : BASE_PARSER(admin, tag),
              m_users(users),
              m_store(store),
              m_tariffs(tariffs),
              m_cashMustBeAdded(false) {}

        int Start(void * data, const char * el, const char ** attr) override;

    private:
        Users & m_users;
        Store & m_store;
        const Tariffs & m_tariffs;
        UserStatOpt m_usr;
        UserConfOpt m_ucr;
        std::optional<uint64_t> m_upr[DIR_NUM];
        std::optional<uint64_t> m_downr[DIR_NUM];
        std::string m_cashMsg;
        std::string m_login;
        bool m_cashMustBeAdded;

        int ApplyChanges();
        void CreateAnswer() override;
};

class DEL_USER: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(Users & users) : m_users(users) {}
                BASE_PARSER * create(const Admin & admin) override { return new DEL_USER(admin, m_users); }
                static void Register(REGISTRY & registry, Users & users)
                { registry[ToLower(tag)] = new FACTORY(users); }
            private:
                Users & m_users;
        };

        static const char * tag;

        DEL_USER(const Admin & admin, Users & users)
            : BASE_PARSER(admin, tag), m_users(users), res(0), u(NULL) {}
        int Start(void * data, const char * el, const char ** attr) override;
        int End(void * data, const char * el) override;

    private:
        Users & m_users;
        int res;
        User * u;

        void CreateAnswer() override;
};

class CHECK_USER: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(const Users & users) : m_users(users) {}
                BASE_PARSER * create(const Admin & admin) override { return new CHECK_USER(admin, m_users); }
                static void Register(REGISTRY & registry, const Users & users)
                { registry[ToLower(tag)] = new FACTORY(users); }
            private:
                const Users & m_users;
        };

        static const char * tag;

        CHECK_USER(const Admin & admin, const Users & users)
            : BASE_PARSER(admin, tag), m_users(users) {}
        int Start(void * data, const char * el, const char ** attr) override;
        int End(void * data, const char * el) override;

    private:
        const Users & m_users;

        void CreateAnswer(const char * error);
        void CreateAnswer() override {} // dummy
};

} // namespace PARSER
} // namespace STG
