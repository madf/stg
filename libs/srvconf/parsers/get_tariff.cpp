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

#include "get_tariff.h"

//#include "parsers/property.h"

#include "stg/common.h"

#include <utility>

#include <strings.h>

using namespace STG;

namespace
{

template <typename A, typename T>
class AoSParser : public BasePropertyParser
{
    public:
        using Func = bool (*)(const char**, A &, T A::value_type::*);
        AoSParser(A& a, T A::value_type::* fld, Func f) : array(a), field(fld), func(f) {}
        virtual bool Parse(const char** attr, const std::string& /*attrName*/, const std::string& /*fromEncoding*/) { return func(attr, array, field); }
    private:
        A& array;
        T A::value_type::* field;
        Func func;
};

template <typename A, typename T>
inline
void addAOSParser(PropertyParsers& parsers, const std::string& name, A& array, T A::value_type::* field, const typename AoSParser<A, T>::Func& func)
{
    parsers.insert(std::make_pair(ToLower(name), new AoSParser<A, T>(array, field, func)));
}

bool getTimeSpan(const char** attr, DirPriceData& value, const std::string& attrName)
{
    if (checkValue(attr, attrName))
    {
        int hb = 0;
        int mb = 0;
        int he = 0;
        int me = 0;
        if (ParseTariffTimeStr(attr[1], hb, mb, he, me) == 0)
        {
            value.hDay = hb;
            value.mDay = mb;
            value.hNight = he;
            value.mNight = me;
            return true;
        }
    }
    return false;
}

template <typename T>
bool getTraffType(const char** attr, T& value, const std::string& attrName)
{
    if (!checkValue(attr, attrName))
        return false;
    value = Tariff::parseTraffType(attr[1]);
    return true;
}

template <typename T>
bool getPeriod(const char** attr, T& value, const std::string& attrName)
{
    if (!checkValue(attr, attrName))
        return false;
    std::string type(attr[1]);
    if (type == "day")
        value = Tariff::DAY;
    else if (type == "month")
        value = Tariff::MONTH;
    else
        return false;
    return true;
}

template <typename T>
bool getChangePolicy(const char** attr, T& value, const std::string& attrName)
{
    if (!checkValue(attr, attrName))
        return false;
    std::string type(attr[1]);
    if (type == "allow")
        value = Tariff::ALLOW;
    else if (type == "to_cheap")
        value = Tariff::TO_CHEAP;
    else if (type == "to_expensive")
        value = Tariff::TO_EXPENSIVE;
    else if (type == "deny")
        value = Tariff::DENY;
    else
        return false;
    return true;
}

template <typename A, typename T>
bool getSlashedValue(const char** attr, A& array, T A::value_type::* field)
{
    if (!checkValue(attr, "value"))
        return false;
    const char* start = attr[1];
    size_t item = 0;
    const char* pos = NULL;
    while ((pos = strchr(start, '/')) && item < array.size())
    {
        if (str2x(std::string(start, pos), array[item++].*field))
                return false;
        start = pos + 1;
    }
    if (item < array.size())
        if (str2x(start, array[item].*field))
            return false;
    return true;
}

} // namespace anonymous

GetTariff::Parser::Parser(Callback f, void* d, const std::string& e)
    : callback(f),
      data(d),
      encoding(e),
      depth(0),
      parsingAnswer(false)
{
    addParser(propertyParsers, "fee", info.tariffConf.fee);
    addParser(propertyParsers, "passiveCost", info.tariffConf.passiveCost);
    addParser(propertyParsers, "free", info.tariffConf.free);
    addParser(propertyParsers, "traffType", info.tariffConf.traffType, getTraffType);
    addParser(propertyParsers, "period", info.tariffConf.period, getPeriod);
    addParser(propertyParsers, "changePolicy", info.tariffConf.changePolicy, getChangePolicy);
    addParser(propertyParsers, "changePolicyTimeout", info.tariffConf.changePolicyTimeout);
    for (size_t i = 0; i < DIR_NUM; ++i)
        addParser(propertyParsers, "time" + std::to_string(i), info.dirPrice[i], getTimeSpan);
    addAOSParser(propertyParsers, "priceDayA", info.dirPrice, &DirPriceData::priceDayA, getSlashedValue);
    addAOSParser(propertyParsers, "priceDayB", info.dirPrice, &DirPriceData::priceDayB, getSlashedValue);
    addAOSParser(propertyParsers, "priceNightA", info.dirPrice, &DirPriceData::priceNightA, getSlashedValue);
    addAOSParser(propertyParsers, "priceNightB", info.dirPrice, &DirPriceData::priceNightB, getSlashedValue);
    addAOSParser(propertyParsers, "singlePrice", info.dirPrice, &DirPriceData::singlePrice, getSlashedValue);
    addAOSParser(propertyParsers, "noDiscount", info.dirPrice, &DirPriceData::noDiscount, getSlashedValue);
    addAOSParser(propertyParsers, "threshold", info.dirPrice, &DirPriceData::threshold, getSlashedValue);
}
//-----------------------------------------------------------------------------
GetTariff::Parser::~Parser()
{
    auto it = propertyParsers.begin();
    while (it != propertyParsers.end())
        delete (it++)->second;
}
//-----------------------------------------------------------------------------
int GetTariff::Parser::ParseStart(const char* el, const char** attr)
{
    depth++;
    if (depth == 1)
        ParseTariff(el, attr);

    if (depth == 2 && parsingAnswer)
        ParseTariffParams(el, attr);

    return 0;
}
//-----------------------------------------------------------------------------
void GetTariff::Parser::ParseEnd(const char* /*el*/)
{
    depth--;
    if (depth == 0 && parsingAnswer)
    {
        if (callback)
            callback(error.empty(), error, info, data);
        error.clear();
        parsingAnswer = false;
    }
}
//-----------------------------------------------------------------------------
void GetTariff::Parser::ParseTariff(const char* el, const char** attr)
{
    if (strcasecmp(el, "tariff") == 0)
    {
        if (attr && attr[0] && attr[1])
        {
            if (strcasecmp(attr[1], "error") == 0)
            {
                if (attr[2] && attr[3])
                    error = attr[3];
                else
                    error = "Tariff not found.";
            }
            else
            {
                parsingAnswer = true;
                if (strcasecmp(attr[0], "name") == 0)
                    info.tariffConf.name = attr[1];
            }
        }
        else
            parsingAnswer = true;
    }
}
//-----------------------------------------------------------------------------
void GetTariff::Parser::ParseTariffParams(const char* el, const char** attr)
{
    if (!tryParse(propertyParsers, ToLower(el), attr, encoding))
        error = std::string("Invalid parameter '") + el + "'.";
}
