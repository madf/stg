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

#include "get_users.h"

#include <cstddef>

#include <strings.h>

using namespace STG;

GET_USERS::PARSER::PARSER()
    : callback(NULL),
      data(NULL),
      depth(0),
      parsingAnswer(false)
{
    userParser.SetCallback(&GET_USERS::PARSER::UserCallback, this);
}
//-----------------------------------------------------------------------------
int GET_USERS::PARSER::ParseStart(const char * el, const char ** attr)
{
depth++;
if (depth == 1 && strcasecmp(el, "users") == 0)
    parsingAnswer = true;

if (depth > 1 && parsingAnswer)
    userParser.ParseStart(el, attr);

return 0;
}
//-----------------------------------------------------------------------------
void GET_USERS::PARSER::ParseEnd(const char * el)
{
depth--;
if (depth > 0 && parsingAnswer)
    userParser.ParseEnd(el);

if (depth == 0 && parsingAnswer)
    {
    if (callback)
        callback(error.empty(), error, info, data);
    error.clear();
    info.clear();
    parsingAnswer = false;
    }
}
//-----------------------------------------------------------------------------
void GET_USERS::PARSER::AddUser(const GET_USER::INFO & userInfo)
{
info.push_back(userInfo);
}
//-----------------------------------------------------------------------------
void GET_USERS::PARSER::SetCallback(CALLBACK f, void * d)
{
callback = f;
data = d;
}
//-----------------------------------------------------------------------------
void GET_USERS::PARSER::UserCallback(bool result, const std::string & error, const GET_USER::INFO & info, void * data)
{
    GET_USERS::PARSER * parser = static_cast<GET_USERS::PARSER *>(data);
    if (!result)
        parser->SetError(error);
    else
        parser->AddUser(info);
}
