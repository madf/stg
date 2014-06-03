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

#include "simple.h"

#include <strings.h>

using namespace STG;

SIMPLE::PARSER::PARSER(const std::string & t, CALLBACK f, void * d, const std::string & e)
    : tag(t),
      callback(f),
      data(d),
      encoding(e),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int SIMPLE::PARSER::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    if (strcasecmp(el, tag.c_str()) == 0)
        ParseAnswer(el, attr);
return 0;
}
//-----------------------------------------------------------------------------
void SIMPLE::PARSER::ParseEnd(const char *)
{
depth--;
}
//-----------------------------------------------------------------------------
void SIMPLE::PARSER::ParseAnswer(const char * /*el*/, const char ** attr)
{
if (!callback)
    return;
if (attr && attr[0] && attr[1])
    callback(strcasecmp(attr[1], "ok") == 0, attr[2] && attr[3] ? attr[3] : attr[1], data);
else
    callback(false, "Invalid response.", data);
}
