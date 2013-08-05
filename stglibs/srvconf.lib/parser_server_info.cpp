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

#include "stg/parser_server_info.h"

#include "stg/common.h"

#include <cstdio> // sprintf

#include <strings.h>

namespace
{

const size_t UNAME_LEN    = 256;
const size_t SERV_VER_LEN = 64;
const size_t DIRNAME_LEN  = 16;

bool checkValue(const char ** attr)
{
return attr && attr[0] && attr[1] && strcasecmp(attr[0], "value") == 0;
}

int getIntValue(const char ** attr)
{
int value = -1;
if (checkValue(attr))
    if (str2x(attr[1], value) < 0)
        return -1;
return value;
}

std::string getStringValue(const char ** attr)
{
if (checkValue(attr))
    return attr[1];
return "";
}

}

PARSER_SERVER_INFO::PARSER_SERVER_INFO()
    : callback(NULL),
      data(NULL),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int PARSER_SERVER_INFO::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    {
    if (strcasecmp(el, "ServerInfo") != 0)
        {
        //printf("%s\n", el);
        }
    }
else
    {
    if (depth == 2)
        {
        if (strcasecmp(el, "uname") == 0)
            {
            info.uname = getStringValue(attr);
            return 0;
            }
        if (strcasecmp(el, "version") == 0)
            {
            info.version = getStringValue(attr);
            return 0;
            }
        if (strcasecmp(el, "tariff") == 0)
            {
            info.tariffType = getIntValue(attr);
            return 0;
            }
        if (strcasecmp(el, "dir_num") == 0)
            {
            info.dirNum = getIntValue(attr);
            return 0;
            }
        if (strcasecmp(el, "users_num") == 0)
            {
            info.usersNum = getIntValue(attr);
            return 0;
            }
        if (strcasecmp(el, "tariff_num") == 0)
            {
            info.tariffNum = getIntValue(attr);
            return 0;
            }

        for (int j = 0; j < DIR_NUM; j++)
            {
            char str[16];
            sprintf(str, "dir_name_%d", j);
            if (strcasecmp(el, str) == 0)
                ParseDirName(attr, j);
            }

        }
    }
return 0;
}
//-----------------------------------------------------------------------------
void PARSER_SERVER_INFO::ParseEnd(const char * /*el*/)
{
depth--;
if (depth == 0 && callback)
    callback(info, data);
}
//-----------------------------------------------------------------------------
void PARSER_SERVER_INFO::SetCallback(CALLBACK f, void * d)
{
callback = f;
data = d;
}
//-----------------------------------------------------------------------------
void PARSER_SERVER_INFO::ParseDirName(const char **attr, int d)
{
if (checkValue(attr))
    {
    char str[2 * DIRNAME_LEN + 1];
    Decode21(str, attr[1]);
    info.dirName[d] = str;
    }
}
