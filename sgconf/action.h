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

#ifndef __STG_SGCONF_ACTION_H__
#define __STG_SGCONF_ACTION_H__

#include <string>
#include <map>
#include <stdexcept>

namespace SGCONF
{

class OPTION_BLOCK;
struct PARSER_STATE;
struct CONFIG;

class ACTION
{
    public:
        virtual ~ACTION() {}

        virtual ACTION * Clone() const = 0;
        virtual std::string ParamDescription() const = 0;
        virtual std::string DefaultDescription() const = 0;
        virtual OPTION_BLOCK & Suboptions() = 0;
        virtual PARSER_STATE Parse(int argc, char ** argv, void * data = NULL) = 0;
        virtual void ParseValue(const std::string &) {}

        class ERROR : public std::runtime_error
        {
            public:
                ERROR(const std::string & message)
                    : std::runtime_error(message.c_str()) {}
        };
};

} // namespace SGCONF

#endif
