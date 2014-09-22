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

#include "get_admin.h"

#include "stg/common.h"

#include <map>
#include <utility>

#include <strings.h>

using namespace STG;

namespace STG
{

template <>
inline
bool GetValue<PRIV>(const char ** attr, PRIV & value, const std::string & attrName)
{
uint32_t priv;
if (!GetValue(attr, priv, attrName))
    return false;
value = priv;
return true;
}

} // namespace STG

GET_ADMIN::PARSER::PARSER(CALLBACK f, void * d, const std::string & e)
    : callback(f),
      data(d),
      encoding(e),
      depth(0),
      parsingAnswer(false)
{
    AddParser(propertyParsers, "login", info.login);
    AddParser(propertyParsers, "password", info.password);
    AddParser(propertyParsers, "priv", info.priv);
}
//-----------------------------------------------------------------------------
GET_ADMIN::PARSER::~PARSER()
{
    PROPERTY_PARSERS::iterator it(propertyParsers.begin());
    while (it != propertyParsers.end())
        delete (it++)->second;
}
//-----------------------------------------------------------------------------
int GET_ADMIN::PARSER::ParseStart(const char * el, const char ** attr)
{
depth++;
if (depth == 1)
    ParseAdmin(el, attr);

/*if (depth == 2 && parsingAnswer)
    ParseAdminParams(el, attr);*/

return 0;
}
//-----------------------------------------------------------------------------
void GET_ADMIN::PARSER::ParseEnd(const char * /*el*/)
{
depth--;
if (depth == 0 && parsingAnswer)
    {
    if (callback)
        callback(error.empty(), error, info, data);
    error.clear();
    parsingAnswer = false;
    }
}
//-----------------------------------------------------------------------------
void GET_ADMIN::PARSER::ParseAdmin(const char * el, const char ** attr)
{
if (strcasecmp(el, "admin") == 0)
    {
    if (attr && attr[0] && attr[1])
        {
        if (strcasecmp(attr[1], "error") == 0)
            {
            if (attr[2] && attr[3])
                error = attr[3];
            else
                error = "Admin not found.";
            }
        else
            {
            parsingAnswer = true;
            for (const char ** pos = attr; *pos != NULL; pos = pos + 2)
                if (!TryParse(propertyParsers, ToLower(*pos), pos, encoding, *pos))
                    {
                    error = std::string("Invalid parameter '") + *pos + "'.";
                    break;
                    }
            }
        }
    else
        parsingAnswer = true;
    }
}
//-----------------------------------------------------------------------------
/*void GET_ADMIN::PARSER::ParseAdminParams(const char * el, const char ** attr)
{
if (!TryParse(propertyParsers, ToLower(el), attr))
    error = "Invalid parameter.";
}*/
