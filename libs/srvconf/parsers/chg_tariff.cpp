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

#include "optional_utils.h"

#include "stg/tariff_conf.h"
#include "stg/common.h"

#include <sstream>

#include <strings.h>

using namespace STG;

namespace
{

template <typename A, typename T>
void appendSlashedResetable(std::ostream& stream, const std::string& name, const A& array, T A::value_type::* field)
{
    std::string res;
    for (typename A::size_type i = 0; i < array.size(); ++i)
    {
        if (!(array[i].*field)) // All values must be set
            return;
        if (!res.empty())
            res += "/";
        res += std::to_string((array[i].*field).value());
    }
    stream << "<" << name << " value=\"" << res << "\"/>";
}

} // namespace anonymous

std::string ChgTariff::serialize(const TariffDataOpt& data, const std::string& /*encoding*/)
{
    std::ostringstream stream;

    appendResetableTag(stream, "fee", data.tariffConf.fee);
    appendResetableTag(stream, "passiveCost", data.tariffConf.passiveCost);
    appendResetableTag(stream, "free", data.tariffConf.free);

    if (data.tariffConf.traffType)
        stream << "<traffType value=\"" + Tariff::toString(data.tariffConf.traffType.value()) + "\"/>";

    if (data.tariffConf.period)
        switch (data.tariffConf.period.value())
        {
            case Tariff::DAY: stream << "<period value=\"day\"/>"; break;
            case Tariff::MONTH: stream << "<period value=\"month\"/>"; break;
        }

    if (data.tariffConf.changePolicy)
        switch (data.tariffConf.changePolicy.value())
        {
            case Tariff::ALLOW: stream << "<changePolicy value=\"allow\"/>"; break;
            case Tariff::TO_CHEAP: stream << "<changePolicy value=\"to_cheap\"/>"; break;
            case Tariff::TO_EXPENSIVE: stream << "<changePolicy value=\"to_expensive\"/>"; break;
            case Tariff::DENY: stream << "<changePolicy value=\"deny\"/>"; break;
        }

    appendResetableTag(stream, "changePolicyTimeout", data.tariffConf.changePolicyTimeout);
    for (size_t i = 0; i < DIR_NUM; ++i)
        if (data.dirPrice[i].hDay ||
            data.dirPrice[i].mDay ||
            data.dirPrice[i].hNight ||
            data.dirPrice[i].mNight)
            stream << "<time" << i << " value=\"" << data.dirPrice[i].hDay.value() << ":"
                                                  << data.dirPrice[i].mDay.value() << "-"
                                                  << data.dirPrice[i].hNight.value() << ":"
                                                  << data.dirPrice[i].mNight.value() << "\"/>";

    appendSlashedResetable(stream, "priceDayA", data.dirPrice, &DirPriceDataOpt::priceDayA);
    appendSlashedResetable(stream, "priceDayB", data.dirPrice, &DirPriceDataOpt::priceDayB);
    appendSlashedResetable(stream, "priceNightA", data.dirPrice, &DirPriceDataOpt::priceNightA);
    appendSlashedResetable(stream, "priceNightB", data.dirPrice, &DirPriceDataOpt::priceNightB);
    appendSlashedResetable(stream, "singlePrice", data.dirPrice, &DirPriceDataOpt::singlePrice);
    appendSlashedResetable(stream, "noDiscount", data.dirPrice, &DirPriceDataOpt::noDiscount);
    appendSlashedResetable(stream, "threshold", data.dirPrice, &DirPriceDataOpt::threshold);

    return stream.str();
}
