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

#include "stg/parser_get_users.h"

#include <cstddef>

#include <strings.h>

PARSER_GET_USERS::PARSER_GET_USERS()
    : callback(NULL),
      data(NULL),
      depth(0)
{
    userParser.SetCallback(&PARSER_GET_USERS::UserCallback, this);
}
//-----------------------------------------------------------------------------
int PARSER_GET_USERS::ParseStart(const char * el, const char ** attr)
{
depth++;
if (depth == 1)
    ParseUsers(el, attr);

if (depth > 1)
    userParser.ParseStart(el, attr);

return 0;
}
//-----------------------------------------------------------------------------
void PARSER_GET_USERS::ParseEnd(const char * el)
{
depth--;
if (depth > 0)
    userParser.ParseEnd(el);

if (depth == 0)
    {
    if (callback)
        callback(error.empty(), error, info, data);
    error.clear();
    }
}
//-----------------------------------------------------------------------------
void PARSER_GET_USERS::ParseUsers(const char * el, const char ** /*attr*/)
{
if (strcasecmp(el, "users") == 0)
    info.clear();
}
//-----------------------------------------------------------------------------
void PARSER_GET_USERS::AddUser(const PARSER_GET_USER::INFO & userInfo)
{
info.push_back(userInfo);
}
//-----------------------------------------------------------------------------
void PARSER_GET_USERS::SetCallback(CALLBACK f, void * d)
{
callback = f;
data = d;
}
//-----------------------------------------------------------------------------
void PARSER_GET_USERS::UserCallback(bool result, const std::string & error, const PARSER_GET_USER::INFO & info, void * data)
{
    PARSER_GET_USERS * parser = static_cast<PARSER_GET_USERS *>(data);
    if (!result)
        parser->SetError(error);
    else
        parser->AddUser(info);
}
