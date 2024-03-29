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

#include "stg/service_conf.h"

#include "stg/common.h"

#include <string>

namespace STG
{

struct Services;

namespace PARSER
{

class GET_SERVICES: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(const Services & services) : m_services(services) {}
                BASE_PARSER * create(const Admin & admin) override { return new GET_SERVICES(admin, m_services); }
                static void Register(REGISTRY & registry, const Services & services)
                { registry[ToLower(tag)] = new FACTORY(services); }
            private:
                const Services & m_services;
        };

        static const char * tag;

        GET_SERVICES(const Admin & admin, const Services & services)
            : BASE_PARSER(admin, tag), m_services(services) {}

    private:
        const Services & m_services;

        void CreateAnswer() override;
};

class GET_SERVICE: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(const Services & services) : m_services(services) {}
                BASE_PARSER * create(const Admin & admin) override { return new GET_SERVICE(admin, m_services); }
                static void Register(REGISTRY & registry, Services & services)
                { registry[ToLower(tag)] = new FACTORY(services); }
            private:
                const Services & m_services;
        };

        static const char * tag;

        GET_SERVICE(const Admin & admin, const Services & services)
            : BASE_PARSER(admin, tag), m_services(services) {}
        int Start(void * data, const char * el, const char ** attr) override;

    private:
        std::string m_name;
        const Services & m_services;

        void CreateAnswer() override;
};

class ADD_SERVICE: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(Services & services) : m_services(services) {}
                BASE_PARSER * create(const Admin & admin) override { return new ADD_SERVICE(admin, m_services); }
                static void Register(REGISTRY & registry, Services & services)
                { registry[ToLower(tag)] = new FACTORY(services); }
            private:
                Services & m_services;
        };

        static const char * tag;

        ADD_SERVICE(const Admin & admin, Services & services)
            : BASE_PARSER(admin, tag), m_services(services) {}
        int Start(void * data, const char * el, const char ** attr) override;

    private:
        std::string m_name;
        Services & m_services;

        void CreateAnswer() override;
};

class DEL_SERVICE: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(Services & services) : m_services(services) {}
                BASE_PARSER * create(const Admin & admin) override { return new DEL_SERVICE(admin, m_services); }
                static void Register(REGISTRY & registry, Services & services)
                { registry[ToLower(tag)] = new FACTORY(services); }
            private:
                Services & m_services;
        };

        static const char * tag;

        DEL_SERVICE(const Admin & admin, Services & services)
            : BASE_PARSER(admin, tag), m_services(services) {}
        int Start(void * data, const char * el, const char ** attr) override;

    private:
        std::string m_name;
        Services & m_services;

        void CreateAnswer() override;
};

class CHG_SERVICE: public BASE_PARSER
{
    public:
        class FACTORY : public BASE_PARSER::FACTORY
        {
            public:
                explicit FACTORY(Services & services) : m_services(services) {}
                BASE_PARSER * create(const Admin & admin) override { return new CHG_SERVICE(admin, m_services); }
                static void Register(REGISTRY & registry, Services & services)
                { registry[ToLower(tag)] = new FACTORY(services); }
            private:
                Services & m_services;
        };

        static const char * tag;

        CHG_SERVICE(const Admin & admin, Services & services)
            : BASE_PARSER(admin, tag), m_services(services) {}
        int Start(void * data, const char * el, const char ** attr) override;

    private:
        ServiceConfOpt m_service;
        Services & m_services;

        void CreateAnswer() override;
};

} // namespace PARSER
} // namespace STG
