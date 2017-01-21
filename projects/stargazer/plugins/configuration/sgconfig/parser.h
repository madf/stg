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
 */

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <map>

class ADMIN;

class BASE_PARSER
{
    public:
        struct FACTORY
        {
            virtual ~FACTORY() {}
            virtual BASE_PARSER * create(const ADMIN & admin) = 0;
        };
        typedef std::map<std::string, FACTORY *> REGISTRY;

        BASE_PARSER(const ADMIN & admin, const std::string & t)
            : m_currAdmin(admin),
              m_depth(0),
              m_tag(t)
        {}
        virtual ~BASE_PARSER() {}
        virtual int Start(void * data, const char * el, const char ** attr);
        virtual int End(void * data, const char * el);

        const std::string & GetAnswer() const { return m_answer; }
        const std::string & GetTag() const { return m_tag; }
        std::string GetOpenTag() const { return "<" + m_tag + ">"; }
        std::string GetCloseTag() const { return "</" + m_tag + ">"; }

    protected:
        BASE_PARSER(const BASE_PARSER & rvalue);
        BASE_PARSER & operator=(const BASE_PARSER & rvalue);

        const ADMIN & m_currAdmin;
        size_t        m_depth;
        std::string   m_answer;
        std::string   m_tag;

    private:
        virtual void CreateAnswer() = 0;
};

#endif //PARSER_H
