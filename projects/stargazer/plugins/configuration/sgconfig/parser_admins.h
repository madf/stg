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

#ifndef __STG_SGCONFIG_PARSER_ADMINS_H__
#define __STG_SGCONFIG_PARSER_ADMINS_H__

#include "parser.h"

#include "stg/resetable.h"

#include <string>

class ADMINS;
class ADMIN;

namespace STG
{
namespace PARSER
{

class GET_ADMINS: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                FACTORY(const ADMIN & admin, const ADMINS & admins)
                    : m_admin(admin), m_admins(admins)
                {}
                virtual BASE_PARSER * create() { return new GET_ADMINS(m_admin, m_admins); }
            private:
                const ADMIN & m_admin;
                const ADMINS & m_admins;
        };

        GET_ADMINS(const ADMIN & admin, const ADMINS & admins)
            : BASE_PARSER(admin, "GetAdmins"), m_admins(admins) {}

    private:
        const ADMINS & m_admins;

        void CreateAnswer();
};

class ADD_ADMIN: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                FACTORY(const ADMIN & admin, ADMINS & admins)
                    : m_admin(admin), m_admins(admins)
                {}
                virtual BASE_PARSER * create() { return new ADD_ADMIN(m_admin, m_admins); }
            private:
                const ADMIN & m_admin;
                ADMINS & m_admins;
        };

        ADD_ADMIN(const ADMIN & admin, ADMINS & admins)
            : BASE_PARSER(admin, "AddAdmin"), m_admins(admins) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        std::string m_admin;
        ADMINS & m_admins;

        void CreateAnswer();
};

class DEL_ADMIN: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                FACTORY(const ADMIN & admin, ADMINS & admins)
                    : m_admin(admin), m_admins(admins)
                {}
                virtual BASE_PARSER * create() { return new DEL_ADMIN(m_admin, m_admins); }
            private:
                const ADMIN & m_admin;
                ADMINS & m_admins;
        };

        DEL_ADMIN(const ADMIN & admin, ADMINS & admins)
            : BASE_PARSER(admin, "DelAdmin"), m_admins(admins) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        std::string m_admin;
        ADMINS & m_admins;

        void CreateAnswer();
};

class CHG_ADMIN: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                FACTORY(const ADMIN & admin, ADMINS & admins)
                    : m_admin(admin), m_admins(admins)
                {}
                virtual BASE_PARSER * create() { return new CHG_ADMIN(m_admin, m_admins); }
            private:
                const ADMIN & m_admin;
                ADMINS & m_admins;
        };

        CHG_ADMIN(const ADMIN & admin, ADMINS & admins)
            : BASE_PARSER(admin, "ChgAdmin"), m_admins(admins) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        std::string login;
        RESETABLE<std::string> password;
        RESETABLE<std::string> privAsString;
        ADMINS & m_admins;

        void CreateAnswer();
};

} // namespace PARSER
} // namespace STG

#endif
