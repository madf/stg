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

std::string ChgAdmin::serialize(const AdminConfOpt& conf, const std::string& /*encoding*/)
{
    std::string params;
    if (conf.login)
        params += " login=\"" + conf.login.value() + "\"";
    if (conf.password)
        params += " password=\"" + conf.password.value() + "\"";
    if (conf.priv)
        params += " priv=\"" + std::to_string(conf.priv.value().toInt()) + "\"";
    return params;
}
