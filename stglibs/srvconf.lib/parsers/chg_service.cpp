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

#include "chg_service.h"

#include "resetable_utils.h"

#include "stg/service_conf.h"
#include "stg/common.h"

#include <sstream>

using namespace STG;

std::string CHG_SERVICE::Serialize(const SERVICE_CONF_RES & conf, const std::string & /*encoding*/)
{
std::ostringstream stream;

appendResetableAttr(stream, "name", conf.name);
appendResetableAttr(stream, "comment", MaybeEncode(conf.comment));
appendResetableAttr(stream, "cost", conf.cost);
appendResetableAttr(stream, "payDay", conf.payDay);

return stream.str();
}
