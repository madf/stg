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

#ifndef __STG_SGCONF_OPTIONS_H__
#define __STG_SGCONF_OPTIONS_H__

#include <string>
#include <vector>
#include <stdexcept>
#include <cstddef> // size_t

namespace SGCONF
{

class ACTION;
struct PARSER_STATE;

class OPTION
{
    public:
        OPTION(const std::string & shortName,
               const std::string & longName,
               ACTION * action,
               const std::string & description);
        OPTION(const std::string & longName,
               ACTION * action,
               const std::string & description);
        OPTION(const OPTION & rhs);
        ~OPTION();

        OPTION & operator=(const OPTION & rhs);

        void Help(size_t level = 0) const;
        PARSER_STATE Parse(int argc, char ** argv);
        bool Check(const char * arg) const;

        class ERROR : public std::runtime_error
        {
            public:
                ERROR(const std::string & message)
                    : std::runtime_error(message.c_str()) {}
        };

    private:
        std::string m_shortName;
        std::string m_longName;
        ACTION * m_action;
        std::string m_description;
};

class OPTION_BLOCK
{
    public:
        void Add(const std::string & shortName,
                 const std::string & longName,
                 ACTION * action,
                 const std::string & description);
        void Add(const std::string & longName,
                 ACTION * action,
                 const std::string & description);

        void Help(size_t level) const;

        PARSER_STATE Parse(int argc, char ** argv);

    private:
        std::vector<OPTION> m_options;
};

} // namespace SGCONF

#endif
