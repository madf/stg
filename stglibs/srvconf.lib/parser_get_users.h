
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

#ifndef __STG_STGLIBS_SRVCONF_PARSER_GET_USERS_H__
#define __STG_STGLIBS_SRVCONF_PARSER_GET_USERS_H__

#include "stg/parser.h"
#include "stg/servconf_types.h"

#include "parser_get_user.h"

#include <vector>

namespace STG
{
namespace GET_USERS
{

class PARSER: public STG::PARSER
{
public:

    PARSER();
    int  ParseStart(const char * el, const char ** attr);
    void ParseEnd(const char * el);
    void SetCallback(CALLBACK f, void * data);
private:
    CALLBACK callback;
    void * data;
    GET_USER::PARSER userParser;
    INFO info;
    int depth;
    bool parsingAnswer;
    std::string error;

    void AddUser(const GET_USER::INFO & userInfo);
    void SetError(const std::string & e) { error = e; }

    static void UserCallback(bool result, const std::string& reason, const GET_USER::INFO & info, void * data);
};

} // namespace GET_USERS
} // namespace STG

#endif
