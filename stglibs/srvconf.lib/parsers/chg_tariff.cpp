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

#include "chg_tariff.h"

#include "resetable_utils.h"

#include "stg/tariff_conf.h"
#include "stg/common.h"

#include <sstream>

#include <strings.h>

using namespace STG;

namespace
{

template <typename A, typename T>
void appendSlashedResetable(std::ostream & stream, const std::string & name, const A & array, T A::value_type:: * field)
{
std::string res;
for (typename A::size_type i = 0; i < array.size(); ++i)
    {
    if ((array[i].*field).empty()) // All values must be set
        return;
    if (!res.empty())
        res += "/";
    res += x2str((array[i].*field).data());
    }
stream << "<" << name << " value=\"" << res << "\"/>";
}

} // namespace anonymous

std::string CHG_TARIFF::Serialize(const TARIFF_DATA_RES & data, const std::string & /*encoding*/)
{
std::ostringstream stream;

appendResetableTag(stream, "fee", data.tariffConf.fee);
appendResetableTag(stream, "passiveCost", data.tariffConf.passiveCost);
appendResetableTag(stream, "free", data.tariffConf.free);

if (!data.tariffConf.traffType.empty())
    stream << "<traffType value=\"" + TARIFF::TraffTypeToString(data.tariffConf.traffType.data()) + "\"/>";

if (!data.tariffConf.period.empty())
    switch (data.tariffConf.period.data())
        {
        case TARIFF::DAY: stream << "<period value=\"day\"/>"; break;
        case TARIFF::MONTH: stream << "<period value=\"month\"/>"; break;
        }

for (size_t i = 0; i < DIR_NUM; ++i)
    if (!data.dirPrice[i].hDay.empty() &&
        !data.dirPrice[i].mDay.empty() &&
        !data.dirPrice[i].hNight.empty() &&
        !data.dirPrice[i].mNight.empty())
        stream << "<time" << i << " value=\"" << data.dirPrice[i].hDay.data() << ":"
                                              << data.dirPrice[i].mDay.data() << "-"
                                              << data.dirPrice[i].hNight.data() << ":"
                                              << data.dirPrice[i].mNight.data() << "\"/>";

appendSlashedResetable(stream, "priceDayA", data.dirPrice, &DIRPRICE_DATA_RES::priceDayA);
appendSlashedResetable(stream, "priceDayB", data.dirPrice, &DIRPRICE_DATA_RES::priceDayB);
appendSlashedResetable(stream, "priceNightA", data.dirPrice, &DIRPRICE_DATA_RES::priceNightA);
appendSlashedResetable(stream, "priceNightB", data.dirPrice, &DIRPRICE_DATA_RES::priceNightB);
appendSlashedResetable(stream, "singlePrice", data.dirPrice, &DIRPRICE_DATA_RES::singlePrice);
appendSlashedResetable(stream, "noDiscount", data.dirPrice, &DIRPRICE_DATA_RES::noDiscount);
appendSlashedResetable(stream, "threshold", data.dirPrice, &DIRPRICE_DATA_RES::threshold);

return stream.str();
}
