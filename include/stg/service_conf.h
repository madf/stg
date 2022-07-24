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

#include "splice.h"

#include <string>
#include <optional>
#include <cstdint>

namespace STG
{

struct ServiceConf
{
    ServiceConf()
        : cost(0), payDay(0)
    {}
    explicit ServiceConf(const std::string & n)
        : name(n), cost(0), payDay(0)
    {}
    ServiceConf(const std::string & n, double c)
        : name(n), cost(c), payDay(0)
    {}
    ServiceConf(const std::string & n, double c, unsigned p)
        : name(n), cost(c), payDay(static_cast<uint8_t>(p))
    {}
    ServiceConf(const std::string & n, double c,
                 unsigned p, const std::string & com)
        : name(n), comment(com), cost(c), payDay(static_cast<uint8_t>(p))
    {}

    ServiceConf(const ServiceConf&) = default;
    ServiceConf& operator=(const ServiceConf&) = default;
    ServiceConf(ServiceConf&&) = default;
    ServiceConf& operator=(ServiceConf&&) = default;

    bool operator==(const ServiceConf& rhs) const noexcept { return name == rhs.name; }

    std::string name;
    std::string comment;
    double      cost;
    uint8_t     payDay;
};

struct ServiceConfOpt
{
    ServiceConfOpt() = default;

    explicit ServiceConfOpt(const ServiceConf& rhs)
        : name(rhs.name), comment(rhs.comment),
          cost(rhs.cost), payDay(rhs.payDay)
    {}

    ServiceConfOpt(const ServiceConfOpt&) = default;
    ServiceConfOpt& operator=(const ServiceConfOpt&) = default;
    ServiceConfOpt(ServiceConfOpt&&) = default;
    ServiceConfOpt& operator=(ServiceConfOpt&&) = default;

    ServiceConfOpt& operator=(const ServiceConf& conf)
    {
        name = conf.name;
        comment = conf.comment;
        cost = conf.cost;
        payDay = conf.payDay;
        return *this;
    }

    void splice(const ServiceConfOpt& rhs)
    {
        STG::splice(name, rhs.name);
        STG::splice(comment, rhs.comment);
        STG::splice(cost, rhs.cost);
        STG::splice(payDay, rhs.payDay);
    }

    ServiceConf get(const ServiceConf& defaultValue) const noexcept
    {
        ServiceConf res;
        res.name = name.value_or(defaultValue.name);
        res.comment = comment.value_or(defaultValue.comment);
        res.cost = cost.value_or(defaultValue.cost);
        res.payDay = payDay.value_or(defaultValue.payDay);
        return res;
    }

    std::optional<std::string> name;
    std::optional<std::string> comment;
    std::optional<double>      cost;
    std::optional<uint8_t>     payDay;
};

}
