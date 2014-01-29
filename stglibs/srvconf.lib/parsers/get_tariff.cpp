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

#include "parsers/property.h"

#include "stg/common.h"

#include <utility>

#include <strings.h>

using namespace STG;

namespace
{

template <typename A, typename T>
class AOS_PARSER : public BASE_PROPERTY_PARSER
{
    public:
        typedef bool (* FUNC)(const char **, A &, T A::value_type:: *);
        AOS_PARSER(A & a, T A::value_type:: * fld, FUNC f) : array(a), field(fld), func(f) {}
        virtual bool Parse(const char ** attr) { return func(attr, array, field); }
    private:
        A & array;
        T A::value_type:: * field;
        FUNC func;
};

template <typename A, typename T>
inline
void AddAOSParser(PROPERTY_PARSERS & parsers, const std::string & name, A & array, T A::value_type:: * field, const typename AOS_PARSER<A, T>::FUNC & func)
{
    parsers.insert(std::make_pair(ToLower(name), new AOS_PARSER<A, T>(array, field, func)));
}

bool GetTimeSpan(const char ** attr, DIRPRICE_DATA & value)
{
int hb = 0;
int mb = 0;
int he = 0;
int me = 0;
if (CheckValue(attr))
    if (ParseTariffTimeStr(attr[1], hb, mb, he, me) == 0)
        {
        value.hDay = hb;
        value.mDay = mb;
        value.hNight = he;
        value.mNight = me;
        return true;
        }
return false;
}

template <typename T>
bool GetTraffType(const char ** attr, T & value)
{
if (!CheckValue(attr))
    return false;
std::string type(attr[1]);
if (type == "up")
    value = TRAFF_UP;
else if (type == "down")
    value = TRAFF_DOWN;
else if (type == "up+down")
    value = TRAFF_UP_DOWN;
else if (type == "max")
    value = TRAFF_MAX;
else
    return false;
return true;
}

template <typename A, typename T>
bool GetSlashedValue(const char ** attr, A & array, T A::value_type:: * field)
{
if (!CheckValue(attr))
    return false;
const char * start = attr[1];
size_t item = 0;
const char * pos = NULL;
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

GET_TARIFF::PARSER::PARSER(CALLBACK f, void * d)
    : callback(f),
      data(d),
      depth(0),
      parsingAnswer(false)
{
    AddParser(propertyParsers, "fee", info.tariffConf.fee);
    AddParser(propertyParsers, "passiveCost", info.tariffConf.passiveCost);
    AddParser(propertyParsers, "free", info.tariffConf.free);
    AddParser(propertyParsers, "traffType", info.tariffConf.traffType, GetTraffType);
    for (size_t i = 0; i < DIR_NUM; ++i)
        AddParser(propertyParsers, "time" + unsigned2str(i), info.dirPrice[i], GetTimeSpan);
    AddAOSParser(propertyParsers, "priceDayA", info.dirPrice, &DIRPRICE_DATA::priceDayA, GetSlashedValue);
    AddAOSParser(propertyParsers, "priceDayB", info.dirPrice, &DIRPRICE_DATA::priceDayB, GetSlashedValue);
    AddAOSParser(propertyParsers, "priceNightA", info.dirPrice, &DIRPRICE_DATA::priceNightA, GetSlashedValue);
    AddAOSParser(propertyParsers, "priceNightB", info.dirPrice, &DIRPRICE_DATA::priceNightB, GetSlashedValue);
    AddAOSParser(propertyParsers, "singlePrice", info.dirPrice, &DIRPRICE_DATA::singlePrice, GetSlashedValue);
    AddAOSParser(propertyParsers, "noDiscount", info.dirPrice, &DIRPRICE_DATA::noDiscount, GetSlashedValue);
    AddAOSParser(propertyParsers, "threshold", info.dirPrice, &DIRPRICE_DATA::threshold, GetSlashedValue);
}
//-----------------------------------------------------------------------------
GET_TARIFF::PARSER::~PARSER()
{
    PROPERTY_PARSERS::iterator it(propertyParsers.begin());
    while (it != propertyParsers.end())
        delete (it++)->second;
}
//-----------------------------------------------------------------------------
int GET_TARIFF::PARSER::ParseStart(const char * el, const char ** attr)
{
depth++;
if (depth == 1)
    ParseTariff(el, attr);

if (depth == 2 && parsingAnswer)
    ParseTariffParams(el, attr);

return 0;
}
//-----------------------------------------------------------------------------
void GET_TARIFF::PARSER::ParseEnd(const char * /*el*/)
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
void GET_TARIFF::PARSER::ParseTariff(const char * el, const char ** attr)
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
            parsingAnswer = true;
        }
    else
        parsingAnswer = true;
    }
}
//-----------------------------------------------------------------------------
void GET_TARIFF::PARSER::ParseTariffParams(const char * el, const char ** attr)
{
if (!TryParse(propertyParsers, ToLower(el), attr))
    error = "Invalid parameter.";
}
