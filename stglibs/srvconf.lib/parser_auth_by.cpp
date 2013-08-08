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

#include "stg/parser_auth_by.h"

#include <cstddef>

#include <strings.h> // strcasecmp

PARSER_AUTH_BY::PARSER_AUTH_BY()
    : callback(NULL),
      data(NULL),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int PARSER_AUTH_BY::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    {
    if (strcasecmp(el, "AuthorizedBy") != 0)
        info.clear();
    }
else
    {
    if (depth == 2)
        {
        if (strcasecmp(el, "Auth") == 0)
            {
            if (attr && attr[0] && attr[1] && strcasecmp(attr[0], "name") == 0)
                info.push_back(attr[1]);
            return 0;
            }
        }
    }
return 0;
}
//-----------------------------------------------------------------------------
void PARSER_AUTH_BY::ParseEnd(const char * /*el*/)
{
depth--;
if (depth == 0 && callback)
    callback(info, data);
}
//-----------------------------------------------------------------------------
void PARSER_AUTH_BY::SetCallback(CALLBACK f, void * d)
{
callback = f;
data = d;
}
