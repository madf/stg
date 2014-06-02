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

#include "get_corp.h"

#include "parsers/property.h"

#include "stg/common.h"

#include <strings.h>

using namespace STG;

GET_CORP::PARSER::PARSER(CALLBACK f, void * d, const std::string & e)
    : callback(f),
      data(d),
      encoding(e),
      depth(0),
      parsingAnswer(false)
{
    AddParser(propertyParsers, "name", info.name);
    AddParser(propertyParsers, "cash", info.cash);
}
//-----------------------------------------------------------------------------
GET_CORP::PARSER::~PARSER()
{
    PROPERTY_PARSERS::iterator it(propertyParsers.begin());
    while (it != propertyParsers.end())
        delete (it++)->second;
}
//-----------------------------------------------------------------------------
int GET_CORP::PARSER::ParseStart(const char * el, const char ** attr)
{
depth++;
if (depth == 1)
    ParseCorp(el, attr);

if (depth == 2 && parsingAnswer)
    ParseCorpParams(el, attr);

return 0;
}
//-----------------------------------------------------------------------------
void GET_CORP::PARSER::ParseEnd(const char * /*el*/)
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
void GET_CORP::PARSER::ParseCorp(const char * el, const char ** attr)
{
if (strcasecmp(el, "corp") == 0)
    {
    if (attr && attr[0] && attr[1])
        {
        if (strcasecmp(attr[1], "error") == 0)
            {
            if (attr[2] && attr[3])
                error = attr[3];
            else
                error = "Corp not found.";
            }
        else
            parsingAnswer = true;
        }
    else
        parsingAnswer = true;
    }
}
//-----------------------------------------------------------------------------
void GET_CORP::PARSER::ParseCorpParams(const char * el, const char ** attr)
{
if (!TryParse(propertyParsers, ToLower(el), attr, encoding))
    error = "Invalid parameter.";
}
