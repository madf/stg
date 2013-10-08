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

#include "chg_admin.h"

#include "stg/admin_conf.h"
#include "stg/common.h"

#include <strings.h>

using namespace STG;

CHG_ADMIN::PARSER::PARSER(SIMPLE::CALLBACK f, void * d)
    : callback(f),
      data(d),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int CHG_ADMIN::PARSER::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    if (strcasecmp(el, "SetAdmin") == 0)
        ParseAnswer(el, attr);
return 0;
}
//-----------------------------------------------------------------------------
void CHG_ADMIN::PARSER::ParseEnd(const char *)
{
depth--;
}
//-----------------------------------------------------------------------------
void CHG_ADMIN::PARSER::ParseAnswer(const char * /*el*/, const char ** attr)
{
if (!callback)
    return;
if (attr && attr[0] && attr[1])
    callback(strcasecmp(attr[1], "ok") == 0, attr[1], data);
else
    callback(false, "Invalid response.", data);
}

std::string CHG_ADMIN::Serialize(const ADMIN_CONF_RES & conf)
{
std::string params;
if (!conf.login.empty())
    params += " login=\"" + conf.login.data() + "\"";
if (!conf.password.empty())
    params += " password=\"" + conf.password.data() + "\"";
if (!conf.priv.empty())
    params += " priv=\"" + x2str(conf.priv.data().ToInt()) + "\"";
return params;
}
