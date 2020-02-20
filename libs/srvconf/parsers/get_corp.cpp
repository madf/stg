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

#include "stg/common.h"

#include <strings.h>

using namespace STG;

GetCorp::Parser::Parser(Callback f, void* d, const std::string& e)
    : callback(f),
      data(d),
      encoding(e),
      depth(0),
      parsingAnswer(false)
{
    addParser(propertyParsers, "name", info.name);
    addParser(propertyParsers, "cash", info.cash);
}
//-----------------------------------------------------------------------------
GetCorp::Parser::~Parser()
{
    auto it = propertyParsers.begin();
    while (it != propertyParsers.end())
        delete (it++)->second;
}
//-----------------------------------------------------------------------------
int GetCorp::Parser::ParseStart(const char* el, const char** attr)
{
    depth++;
    if (depth == 1)
        ParseCorp(el, attr);

    if (depth == 2 && parsingAnswer)
        ParseCorpParams(el, attr);

    return 0;
}
//-----------------------------------------------------------------------------
void GetCorp::Parser::ParseEnd(const char* /*el*/)
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
void GetCorp::Parser::ParseCorp(const char* el, const char** attr)
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
void GetCorp::Parser::ParseCorpParams(const char* el, const char** attr)
{
    if (!tryParse(propertyParsers, ToLower(el), attr, encoding))
        error = "Invalid parameter.";
}
