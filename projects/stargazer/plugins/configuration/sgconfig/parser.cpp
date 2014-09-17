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
 */

#include "parser.h"

#include "stg/tariffs.h"
#include "stg/admin.h"
#include "stg/users.h"
#include "stg/user_property.h"
#include "stg/settings.h"
#include "stg/logger.h"
#include "stg/version.h"
#include "stg/store.h"

#include <cstring>
#include <cstdio> // sprintf

//-----------------------------------------------------------------------------
//  BASE PARSER
//-----------------------------------------------------------------------------
int BASE_PARSER::Start(void *, const char *el, const char **)
{
if (strcasecmp(el, tag.c_str()) == 0)
    return 0;

return -1;
}
//-----------------------------------------------------------------------------
int BASE_PARSER::End(void *, const char *el)
{
if (strcasecmp(el, tag.c_str()) == 0)
    {
    CreateAnswer();
    return 0;
    }

return -1;
}
