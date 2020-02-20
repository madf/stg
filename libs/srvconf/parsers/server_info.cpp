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

#include "server_info.h"

#include "stg/common.h"

#include <cstdio> // sprintf

#include <strings.h>

using namespace STG;

ServerInfo::Parser::Parser(Callback f, void* d, const std::string& e)
    : callback(f),
      data(d),
      encoding(e),
      depth(0),
      parsingAnswer(false)
{
    addParser(propertyParsers, "uname", info.uname);
    addParser(propertyParsers, "version", info.version);
    addParser(propertyParsers, "tariff", info.tariffType);
    addParser(propertyParsers, "dir_num", info.dirNum);
    addParser(propertyParsers, "user_num", info.usersNum);
    addParser(propertyParsers, "tariff_num", info.tariffNum);

    for (size_t i = 0; i < DIR_NUM; i++)
        addParser(propertyParsers, "dir_name_" + std::to_string(i), info.dirName[i], getEncodedValue);
}
//-----------------------------------------------------------------------------
int ServerInfo::Parser::ParseStart(const char* el, const char** attr)
{
    depth++;
    if (depth == 1)
    {
        if (strcasecmp(el, "GetServerInfo") == 0)
            parsingAnswer = true;
    }
    else
    {
        if (depth == 2 && parsingAnswer)
            if (!tryParse(propertyParsers, ToLower(el), attr, encoding))
                error = "Invalid parameter.";
    }
    return 0;
}
//-----------------------------------------------------------------------------
void ServerInfo::Parser::ParseEnd(const char* /*el*/)
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
