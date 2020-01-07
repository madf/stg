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

#ifndef __STG_SGCONFIG_PARSER_SERVICES_H__
#define __STG_SGCONFIG_PARSER_SERVICES_H__

#include "parser.h"

#include "stg/service_conf.h"

#include "stg/common.h"

#include <string>

class SERVICES;

namespace STG
{
namespace PARSER
{

class GET_SERVICES: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                FACTORY(const SERVICES & services) : m_services(services) {}
                virtual BASE_PARSER * create(const ADMIN & admin) { return new GET_SERVICES(admin, m_services); }
                static void Register(REGISTRY & registry, const SERVICES & services)
                { registry[ToLower(tag)] = new FACTORY(services); }
            private:
                const SERVICES & m_services;
        };

        static const char * tag;

        GET_SERVICES(const ADMIN & admin, const SERVICES & services)
            : BASE_PARSER(admin, tag), m_services(services) {}

    private:
        const SERVICES & m_services;

        void CreateAnswer();
};

class GET_SERVICE: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                FACTORY(const SERVICES & services) : m_services(services) {}
                virtual BASE_PARSER * create(const ADMIN & admin) { return new GET_SERVICE(admin, m_services); }
                static void Register(REGISTRY & registry, SERVICES & services)
                { registry[ToLower(tag)] = new FACTORY(services); }
            private:
                const SERVICES & m_services;
        };

        static const char * tag;

        GET_SERVICE(const ADMIN & admin, const SERVICES & services)
            : BASE_PARSER(admin, tag), m_services(services) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        std::string m_name;
        const SERVICES & m_services;

        void CreateAnswer();
};

class ADD_SERVICE: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                FACTORY(SERVICES & services) : m_services(services) {}
                virtual BASE_PARSER * create(const ADMIN & admin) { return new ADD_SERVICE(admin, m_services); }
                static void Register(REGISTRY & registry, SERVICES & services)
                { registry[ToLower(tag)] = new FACTORY(services); }
            private:
                SERVICES & m_services;
        };

        static const char * tag;

        ADD_SERVICE(const ADMIN & admin, SERVICES & services)
            : BASE_PARSER(admin, tag), m_services(services) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        std::string m_name;
        SERVICES & m_services;

        void CreateAnswer();
};

class DEL_SERVICE: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                FACTORY(SERVICES & services) : m_services(services) {}
                virtual BASE_PARSER * create(const ADMIN & admin) { return new DEL_SERVICE(admin, m_services); }
                static void Register(REGISTRY & registry, SERVICES & services)
                { registry[ToLower(tag)] = new FACTORY(services); }
            private:
                SERVICES & m_services;
        };

        static const char * tag;

        DEL_SERVICE(const ADMIN & admin, SERVICES & services)
            : BASE_PARSER(admin, tag), m_services(services) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        std::string m_name;
        SERVICES & m_services;

        void CreateAnswer();
};

class CHG_SERVICE: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                FACTORY(SERVICES & services) : m_services(services) {}
                virtual BASE_PARSER * create(const ADMIN & admin) { return new CHG_SERVICE(admin, m_services); }
                static void Register(REGISTRY & registry, SERVICES & services)
                { registry[ToLower(tag)] = new FACTORY(services); }
            private:
                SERVICES & m_services;
        };

        static const char * tag;

        CHG_SERVICE(const ADMIN & admin, SERVICES & services)
            : BASE_PARSER(admin, tag), m_services(services) {}
        int Start(void * data, const char * el, const char ** attr);

    private:
        SERVICE_CONF_RES m_service;
        SERVICES & m_services;

        void CreateAnswer();
};

} // namespace PARSER
} // namespace STG

#endif
