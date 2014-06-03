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

#include "auth_by.h"

#include <strings.h> // strcasecmp

using namespace STG;

AUTH_BY::PARSER::PARSER(CALLBACK f, void * d, const std::string & e)
    : callback(f),
      data(d),
      encoding(e),
      depth(0),
      parsingAnswer(false)
{
}
//-----------------------------------------------------------------------------
int AUTH_BY::PARSER::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    {
    if (strcasecmp(el, "AuthorizedBy") == 0)
        if (attr && attr[0] && attr[1])
            {
            if (strcasecmp(attr[1], "error") == 0)
                {
                if (attr[2] && attr[3])
                    error = attr[3];
                else
                    error = "User not found.";
                }
            else
                parsingAnswer = true;
            }
    }
else
    {
    if (depth == 2)
        {
        if (parsingAnswer && strcasecmp(el, "Auth") == 0)
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
void AUTH_BY::PARSER::ParseEnd(const char * /*el*/)
{
depth--;
if (depth == 0)
    {
    if (callback)
        callback(error.empty(), error, info, data);
    info.clear();
    error.clear();
    }
}
