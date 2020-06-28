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
 *    Author : Maksym Mamontov <stg@madf.info>
 */

#ifndef __STG_SGCONFIG_PARSER_USERS_H__
#define __STG_SGCONFIG_PARSER_USERS_H__

#include "parser.h"

#include "stg/user_conf.h"
#include "stg/user_stat.h"
#include "stg/common.h"
#include "stg/resetable.h"

#include <string>

class USERS;
class USER;
class TARIFFS;
class ADMIN;
class STORE;

namespace STG
{
namespace PARSER
{

class GET_USERS: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(USERS & users) : m_users(users) {}
                virtual BASE_PARSER * create(const ADMIN & admin) { return new GET_USERS(admin, m_users); }
                static void Register(REGISTRY & registry, USERS & users)
                { registry[ToLower(tag)] = new FACTORY(users); }
            private:
                USERS & m_users;
        };

        static const char * tag;

        GET_USERS(const ADMIN & admin, USERS & users)
            : BASE_PARSER(admin, tag), m_users(users),
              m_lastUserUpdateTime(0) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        USERS & m_users;
        time_t m_lastUserUpdateTime;

        void CreateAnswer();
};

class GET_USER: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(const USERS & users) : m_users(users) {}
                virtual BASE_PARSER * create(const ADMIN & admin) { return new GET_USER(admin, m_users); }
                static void Register(REGISTRY & registry, const USERS & users)
                { registry[ToLower(tag)] = new FACTORY(users); }
            private:
                const USERS & m_users;
        };

        static const char * tag;

        GET_USER(const ADMIN & admin, const USERS & users)
            : BASE_PARSER(admin, tag), m_users(users) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        const USERS & m_users;
        std::string m_login;

        void CreateAnswer();
};

class ADD_USER: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(USERS & users) : m_users(users) {}
                virtual BASE_PARSER * create(const ADMIN & admin) { return new ADD_USER(admin, m_users); }
                static void Register(REGISTRY & registry, USERS & users)
                { registry[ToLower(tag)] = new FACTORY(users); }
            private:
                USERS & m_users;
        };

        static const char * tag;

        ADD_USER(const ADMIN & admin, USERS & users)
            : BASE_PARSER(admin, tag), m_users(users) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        USERS & m_users;
        std::string m_login;

        void CreateAnswer();
};

class CHG_USER: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                FACTORY(USERS & users, STORE & store, const TARIFFS & tariffs)
                    : m_users(users), m_store(store), m_tariffs(tariffs)
                {}
                virtual BASE_PARSER * create(const ADMIN & admin) { return new CHG_USER(admin, m_users, m_store, m_tariffs); }
                static void Register(REGISTRY & registry, USERS & users, STORE & store, const TARIFFS & tariffs)
                { registry[ToLower(tag)] = new FACTORY(users, store, tariffs); }
            private:
                USERS & m_users;
                STORE & m_store;
                const TARIFFS & m_tariffs;
        };

        static const char * tag;

        CHG_USER(const ADMIN & admin, USERS & users,
                 STORE & store, const TARIFFS & tariffs)
            : BASE_PARSER(admin, tag),
              m_users(users),
              m_store(store),
              m_tariffs(tariffs),
              m_cashMustBeAdded(false) {}

        int Start(void * data, const char * el, const char ** attr);

    private:
        USERS & m_users;
        STORE & m_store;
        const TARIFFS & m_tariffs;
        USER_STAT_RES m_usr;
        USER_CONF_RES m_ucr;
        RESETABLE<uint64_t> m_upr[DIR_NUM];
        RESETABLE<uint64_t> m_downr[DIR_NUM];
        std::string m_cashMsg;
        std::string m_login;
        bool m_cashMustBeAdded;

        int ApplyChanges();
        void CreateAnswer();
};

class DEL_USER: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(USERS & users) : m_users(users) {}
                virtual BASE_PARSER * create(const ADMIN & admin) { return new DEL_USER(admin, m_users); }
                static void Register(REGISTRY & registry, USERS & users)
                { registry[ToLower(tag)] = new FACTORY(users); }
            private:
                USERS & m_users;
        };

        static const char * tag;

        DEL_USER(const ADMIN & admin, USERS & users)
            : BASE_PARSER(admin, tag), m_users(users), res(0), u(NULL) {}
        int Start(void * data, const char * el, const char ** attr);
        int End(void * data, const char * el);

    private:
        USERS & m_users;
        int res;
        USER * u;

        void CreateAnswer();
};

class CHECK_USER: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(const USERS & users) : m_users(users) {}
                virtual BASE_PARSER * create(const ADMIN & admin) { return new CHECK_USER(admin, m_users); }
                static void Register(REGISTRY & registry, const USERS & users)
                { registry[ToLower(tag)] = new FACTORY(users); }
            private:
                const USERS & m_users;
        };

        static const char * tag;

        CHECK_USER(const ADMIN & admin, const USERS & users)
            : BASE_PARSER(admin, tag), m_users(users) {}
        int Start(void * data, const char * el, const char ** attr);
        int End(void * data, const char * el);

    private:
        const USERS & m_users;

        void CreateAnswer(const char * error);
        void CreateAnswer() {} // dummy
};

} // namespace PARSER
} // namespace STG

#endif
