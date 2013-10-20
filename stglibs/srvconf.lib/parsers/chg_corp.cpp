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

#include "stg/corp_conf.h"
#include "stg/common.h"

#include <sstream>

using namespace STG;

namespace
{

template <typename T>
void appendResetable(std::ostream & stream, const std::string & name, const T & value)
{
if (!value.empty())
    stream << "<" << name << " value=\"" << value.data() << "\"/>";
}

} // namespace anonymous

std::string CHG_CORP::Serialize(const CORP_CONF_RES & conf)
{
std::ostringstream stream;

appendResetable(stream, "name", conf.name);
appendResetable(stream, "cash", conf.cash);

return stream.str();
}
