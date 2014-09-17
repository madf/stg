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

#ifndef __STG_SGCONFIG_PARSER_USERS_H__
#define __STG_SGCONFIG_PARSER_USERS_H__

#include "parser.h"

#include "stg/resetable.h"

#include <string>

class USERS;
class USER;
class TARIFFS;
class ADMIN;

namespace STG
{
namespace PARSER
{

class GET_USERS: public BASE_PARSER
{
    public:
        GET_USERS(const ADMIN & admin, USERS & users)
            : BASE_PARSER(admin, "GetUsers"), m_users(users),
              lastUserUpdateTime(0), lastUpdateFound(false) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        USERS & m_users;
        time_t lastUserUpdateTime;
        bool lastUpdateFound;

        void CreateAnswer();
};

class GET_USER: public BASE_PARSER
{
    public:
        GET_USER(const ADMIN & admin, const USERS & users)
            : BASE_PARSER(admin, "GetUser"), m_users(users) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        const USERS & m_users;
        std::string login;

        void CreateAnswer();
};

class ADD_USER: public BASE_PARSER
{
    public:
        ADD_USER(const ADMIN & admin, USERS & users)
            : BASE_PARSER(admin, "AddUser"), m_users(users) {}
        int Start(void * data, const char * el, const char ** attr);
        int End(void * data, const char * el);

    private:
        USERS & m_users;
        std::string login;

        int CheckUserData();
        void CreateAnswer();
};

class CHG_USER: public BASE_PARSER
{
    public:
        CHG_USER(const ADMIN & admin, USERS & users, const TARIFFS & tariffs);
        ~CHG_USER();
        int Start(void * data, const char * el, const char ** attr);
        int End(void * data, const char * el);

    private:
        USERS & m_users;
        const TARIFFS & m_tariffs;
        USER_STAT_RES * usr;
        USER_CONF_RES * ucr;
        RESETABLE<uint64_t> * upr;
        RESETABLE<uint64_t> * downr;
        std::string cashMsg;
        std::string login;
        bool cashMustBeAdded;
        int res;

        int ApplyChanges();
        void CreateAnswer();
};

class DEL_USER: public BASE_PARSER
{
    public:
        DEL_USER(const ADMIN & admin, USERS & users)
            : BASE_PARSER(admin, "DelUser"), m_users(users), res(0), u(NULL) {}
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
        CHECK_USER(const ADMIN & admin, const USERS & users)
            : BASE_PARSER(admin, "CheckUser"), m_users(users) {}
        int Start(void * data, const char * el, const char ** attr);
        int End(void * data, const char * el);

    private:
        const USERS & m_users;

        void CreateAnswer(const char * error);
};

}
}

#endif
