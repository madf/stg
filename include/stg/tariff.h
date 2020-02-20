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

#pragma once

#include <string>
//#include <istream>
#include <cstring>
#include <ctime>
#include <cstdint>

namespace STG
{

struct TariffData;

struct Tariff {
    enum ChangePolicy { ALLOW = 0, TO_CHEAP, TO_EXPENSIVE, DENY };

    enum Period { DAY = 0, MONTH };

    enum TraffType { TRAFF_UP = 0, TRAFF_DOWN, TRAFF_UP_DOWN, TRAFF_MAX };

    static std::string toString(ChangePolicy changePolicy);
    static ChangePolicy parseChangePolicy(const std::string& value);

    static std::string toString(Period period);
    static Period parsePeriod(const std::string& value);

    static std::string toString(TraffType type);
    static TraffType parseTraffType(const std::string& value);
    static TraffType fromInt(int value);

    virtual ~Tariff() = default;

    virtual double  GetPriceWithTraffType(uint64_t up,
                                          uint64_t down,
                                          int dir,
                                          time_t t) const = 0;
    virtual double  GetFreeMb() const = 0;
    virtual double  GetPassiveCost() const = 0;
    virtual double  GetFee() const = 0;
    virtual double  GetFree() const = 0;
    virtual Period  GetPeriod() const = 0;
    virtual ChangePolicy GetChangePolicy() const = 0;
    virtual time_t  GetChangePolicyTimeout() const = 0;

    virtual const   std::string& GetName() const = 0;
    virtual void    SetName(const std::string& name) = 0;

    virtual int     GetTraffType() const = 0;
    virtual int64_t GetTraffByType(uint64_t up, uint64_t down) const = 0;
    virtual int     GetThreshold(int dir) const = 0;
    virtual const TariffData& GetTariffData() const = 0;
    virtual std::string TariffChangeIsAllowed(const Tariff& to, time_t currentTime) const = 0;
};

inline
std::string Tariff::toString(ChangePolicy changePolicy)
{
    switch (changePolicy)
    {
        case ALLOW: return "allow";
        case TO_CHEAP: return "to_cheap";
        case TO_EXPENSIVE: return "to_expensive";
        case DENY: return "deny";
    }
    return "allow"; // Classic behaviour.
}

inline
Tariff::ChangePolicy Tariff::parseChangePolicy(const std::string& value)
{
    if (strcasecmp(value.c_str(), "to_cheap") == 0)
        return TO_CHEAP;
    if (strcasecmp(value.c_str(), "to_expensive") == 0)
        return TO_EXPENSIVE;
    if (strcasecmp(value.c_str(), "deny") == 0)
        return DENY;
    return ALLOW; // Classic behaviour.
}

inline
std::string Tariff::toString(Period period)
{
    switch (period)
    {
        case DAY: return "day";
        case MONTH: return "month";
    }
    return "month"; // Classic behaviour.
}

inline
Tariff::Period Tariff::parsePeriod(const std::string& value)
{
    if (strcasecmp(value.c_str(), "day") == 0)
        return DAY;
    return MONTH; // Classic behaviour.
}

inline
std::string Tariff::toString(TraffType type)
{
    switch (type)
    {
        case TRAFF_UP: return "up";
        case TRAFF_DOWN: return "down";
        case TRAFF_UP_DOWN: return "up+down";
        case TRAFF_MAX: return "max";
    }
    return "up+down";
}

inline
Tariff::TraffType Tariff::parseTraffType(const std::string& value)
{
    if (strcasecmp(value.c_str(), "up") == 0)
        return TRAFF_UP;
    if (strcasecmp(value.c_str(), "down") == 0)
        return TRAFF_DOWN;
    if (strcasecmp(value.c_str(), "up+down") == 0)
        return TRAFF_UP_DOWN;
    if (strcasecmp(value.c_str(), "max") == 0)
        return TRAFF_MAX;
    return TRAFF_UP_DOWN;
}

/*inline
std::istream& operator>>(std::istream& stream, Tariff::TraffType& traffType)
{
    unsigned val;
    stream >> val;
    traffType = static_cast<Tariff::TraffType>(val);
    return stream;
}*/

inline
Tariff::TraffType Tariff::fromInt(int value)
{
    if (value < 0 || value > TRAFF_MAX)
        return TRAFF_UP_DOWN;
    return static_cast<TraffType>(value);
}

}
