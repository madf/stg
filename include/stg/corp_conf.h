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

#include "stg/optional.h"

#include <string>

namespace STG
{

struct CorpConf
{
    CorpConf() noexcept : cash(0) {}
    explicit CorpConf(const std::string & n) noexcept : name(n), cash(0) {}
    CorpConf(const std::string & n, double c) noexcept : name(n), cash(c) {}

    CorpConf(const CorpConf&) = default;
    CorpConf& operator=(const CorpConf&) = default;
    CorpConf(CorpConf&&) = default;
    CorpConf& operator=(CorpConf&&) = default;

    bool operator==(const CorpConf& rhs) const noexcept { return name == rhs.name; }

    std::string name;
    double      cash;
};

struct CorpConfOpt
{
    Optional<std::string> name;
    Optional<double>      cash;
};

}
