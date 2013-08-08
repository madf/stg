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

#include "stg/parser_check_user.h"

#include <cstddef>

#include <strings.h>

PARSER_CHECK_USER::PARSER_CHECK_USER()
    : callback(NULL),
      data(NULL),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int PARSER_CHECK_USER::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    if (strcasecmp(el, "CheckUser") == 0)
        ParseAnswer(el, attr);
return 0;
}
//-----------------------------------------------------------------------------
void PARSER_CHECK_USER::ParseEnd(const char *)
{
depth--;
}
//-----------------------------------------------------------------------------
void PARSER_CHECK_USER::ParseAnswer(const char *, const char **attr)
{
if (attr && attr[0] && attr[1] && strcasecmp(attr[0], "value") == 0)
    if (callback)
        callback(attr[1], data);
}
//-----------------------------------------------------------------------------
void PARSER_CHECK_USER::SetCallback(CALLBACK f, void * d)
{
callback = f;
data = d;
}
