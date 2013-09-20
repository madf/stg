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

#include "stg/parser_send_message.h"

#include <cstddef>

#include <strings.h>

using namespace STG;

PARSER_SEND_MESSAGE::PARSER_SEND_MESSAGE()
    : callback(NULL),
      data(NULL),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int PARSER_SEND_MESSAGE::ParseStart(const char * el, const char ** attr)
{
depth++;
if (depth == 1)
    if (strcasecmp(el, "SendMessageResult") == 0)
        ParseAnswer(el, attr);
return 0;
}
//-----------------------------------------------------------------------------
void PARSER_SEND_MESSAGE::ParseEnd(const char * /*el*/)
{
depth--;
}
//-----------------------------------------------------------------------------
void PARSER_SEND_MESSAGE::ParseAnswer(const char * /*el*/, const char **attr)
{
if (!callback)
    return;
if (attr && attr[0] && attr[1])
    callback(strcasecmp(attr[1], "ok") == 0, attr[1], data);
else
    callback(false, "Invalid response.", data);
}
//-----------------------------------------------------------------------------
void PARSER_SEND_MESSAGE::SetCallback(CALLBACK f, void * d)
{
callback = f;
data = d;
}
