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

#include "get_service.h"

#include "parsers/property.h"

#include "stg/common.h"

#include <strings.h>

using namespace STG;

GET_SERVICE::PARSER::PARSER(CALLBACK f, void * d, const std::string & e)
    : callback(f),
      data(d),
      encoding(e),
      depth(0),
      parsingAnswer(false)
{
    AddParser(propertyParsers, "name", info.name);
    AddParser(propertyParsers, "comment", info.comment, GetEncodedValue);
    AddParser(propertyParsers, "cost", info.cost);
    AddParser(propertyParsers, "payDay", info.payDay);
}
//-----------------------------------------------------------------------------
GET_SERVICE::PARSER::~PARSER()
{
    PROPERTY_PARSERS::iterator it(propertyParsers.begin());
    while (it != propertyParsers.end())
        delete (it++)->second;
}
//-----------------------------------------------------------------------------
int GET_SERVICE::PARSER::ParseStart(const char * el, const char ** attr)
{
depth++;
if (depth == 1)
    ParseService(el, attr);

/*if (depth == 2 && parsingAnswer)
    ParseServiceParams(el, attr);*/

return 0;
}
//-----------------------------------------------------------------------------
void GET_SERVICE::PARSER::ParseEnd(const char * /*el*/)
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
void GET_SERVICE::PARSER::ParseService(const char * el, const char ** attr)
{
if (strcasecmp(el, "service") == 0)
    {
    if (attr && attr[0] && attr[1])
        {
        if (strcasecmp(attr[1], "error") == 0)
            {
            if (attr[2] && attr[3])
                error = attr[3];
            else
                error = "Service not found.";
            }
        else
            {
            parsingAnswer = true;
            for (const char ** pos = attr; *pos != NULL; pos = pos + 2)
                if (!TryParse(propertyParsers, ToLower(*pos), pos, encoding, *pos))
                    {
                    error = std::string("Invalid parameter '") + *pos + "' or value '" + *(pos + 1) + "'.";
                    break;
                    }
            }
        }
    else
        parsingAnswer = true;
    }
}
//-----------------------------------------------------------------------------
/*void GET_SERVICE::PARSER::ParseServiceParams(const char * el, const char ** attr)
{
if (!TryParse(propertyParsers, ToLower(el), attr, encoding))
    error = "Invalid parameter.";
}*/
