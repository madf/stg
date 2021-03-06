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

#include "chg_corp.h"

#include "optional_utils.h"

#include "stg/corp_conf.h"
#include "stg/common.h"

#include <sstream>

using namespace STG;

std::string ChgCorp::serialize(const CorpConfOpt& conf, const std::string& /*encoding*/)
{
    std::ostringstream stream;
    appendResetableTag(stream, "name", conf.name);
    appendResetableTag(stream, "cash", conf.cash);
    return stream.str();
}
