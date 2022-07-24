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
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

#pragma once

#include "tariff.h"
#include "const.h"
#include "splice.h"

#include <string>
#include <vector>
#include <optional>

namespace STG
{
//-----------------------------------------------------------------------------
struct DirPriceData
{
    DirPriceData() noexcept
        : hDay(0),
          mDay(0),
          hNight(0),
          mNight(0),
          priceDayA(0),
          priceNightA(0),
          priceDayB(0),
          priceNightB(0),
          threshold(0),
          singlePrice(0),
          noDiscount(0)
    {}

    DirPriceData(const DirPriceData&) = default;
    DirPriceData& operator=(const DirPriceData&) = default;
    DirPriceData(DirPriceData&&) = default;
    DirPriceData& operator=(DirPriceData&&) = default;

    int    hDay;
    int    mDay;
    int    hNight;
    int    mNight;
    double priceDayA;
    double priceNightA;
    double priceDayB;
    double priceNightB;
    int    threshold;
    int    singlePrice; // Do not use day/night division
    int    noDiscount; // Do not use threshold
};
//-----------------------------------------------------------------------------
struct DirPriceDataOpt
{
    DirPriceDataOpt() = default;

    DirPriceDataOpt(const DirPriceData& data) noexcept
        : hDay(data.hDay),
          mDay(data.mDay),
          hNight(data.hNight),
          mNight(data.mNight),
          priceDayA(data.priceDayA),
          priceNightA(data.priceNightA),
          priceDayB(data.priceDayB),
          priceNightB(data.priceNightB),
          threshold(data.threshold),
          singlePrice(data.singlePrice),
          noDiscount(data.noDiscount)
    {}

    DirPriceDataOpt(const DirPriceDataOpt&) = default;
    DirPriceDataOpt& operator=(const DirPriceDataOpt&) = default;
    DirPriceDataOpt(DirPriceDataOpt&&) = default;
    DirPriceDataOpt& operator=(DirPriceDataOpt&&) = default;

    DirPriceDataOpt & operator=(const DirPriceData& rhs) noexcept
    {
        hDay        = rhs.hDay;
        mDay        = rhs.mDay;
        hNight      = rhs.hNight;
        mNight      = rhs.mNight;
        priceDayA   = rhs.priceDayA;
        priceNightA = rhs.priceNightA;
        priceDayB   = rhs.priceDayB;
        priceNightB = rhs.priceNightB;
        threshold   = rhs.threshold;
        singlePrice = rhs.singlePrice;
        noDiscount  = rhs.noDiscount;
        return *this;
    }

    void splice(const DirPriceDataOpt & rhs) noexcept
    {
        STG::splice(hDay, rhs.hDay);
        STG::splice(mDay, rhs.mDay);
        STG::splice(hNight, rhs.hNight);
        STG::splice(mNight, rhs.mNight);
        STG::splice(priceDayA, rhs.priceDayA);
        STG::splice(priceNightA, rhs.priceNightA);
        STG::splice(priceDayB, rhs.priceDayB);
        STG::splice(priceNightB, rhs.priceNightB);
        STG::splice(threshold, rhs.threshold);
        STG::splice(singlePrice, rhs.singlePrice);
        STG::splice(noDiscount, rhs.noDiscount);
    }

    DirPriceData get(const DirPriceData& defaultValue) const noexcept
    {
        DirPriceData res;
        res.hDay = hDay.value_or(defaultValue.hDay);
        res.mDay = mDay.value_or(defaultValue.mDay);
        res.hNight = hNight.value_or(defaultValue.hNight);
        res.mNight = mNight.value_or(defaultValue.mNight);
        res.priceDayA = priceDayA.value_or(defaultValue.priceDayA);
        res.priceNightA = priceNightA.value_or(defaultValue.priceNightA);
        res.priceDayB = priceDayB.value_or(defaultValue.priceDayB);
        res.priceNightB = priceNightB.value_or(defaultValue.priceNightB);
        res.threshold = threshold.value_or(defaultValue.threshold);
        res.singlePrice = singlePrice.value_or(defaultValue.singlePrice);
        res.noDiscount = noDiscount.value_or(defaultValue.noDiscount);
        return res;
    }

    std::optional<int>    hDay;
    std::optional<int>    mDay;
    std::optional<int>    hNight;
    std::optional<int>    mNight;
    std::optional<double> priceDayA;
    std::optional<double> priceNightA;
    std::optional<double> priceDayB;
    std::optional<double> priceNightB;
    std::optional<int>    threshold;
    std::optional<int>    singlePrice;
    std::optional<int>    noDiscount;
};
//-----------------------------------------------------------------------------
struct TariffConf
{
    double             fee;
    double             free;
    Tariff::TraffType  traffType;
    double             passiveCost;
    std::string        name;
    Tariff::Period     period;
    Tariff::ChangePolicy changePolicy;
    time_t changePolicyTimeout;

    TariffConf() noexcept
        : fee(0),
          free(0),
          traffType(Tariff::TRAFF_UP_DOWN),
          passiveCost(0),
          period(Tariff::MONTH),
          changePolicy(Tariff::ALLOW),
          changePolicyTimeout(0)
    {}

    explicit TariffConf(const std::string & n) noexcept
        : fee(0),
          free(0),
          traffType(Tariff::TRAFF_UP_DOWN),
          passiveCost(0),
          name(n),
          period(Tariff::MONTH),
          changePolicy(Tariff::ALLOW),
          changePolicyTimeout(0)
    {}

    TariffConf(const TariffConf&) = default;
    TariffConf& operator=(const TariffConf&) = default;
    TariffConf(TariffConf&&) = default;
    TariffConf& operator=(TariffConf&&) = default;
};
//-----------------------------------------------------------------------------
struct TariffConfOpt
{
    TariffConfOpt() = default;
    TariffConfOpt(const TariffConf& data) noexcept
        : fee(data.fee),
          free(data.free),
          traffType(data.traffType),
          passiveCost(data.passiveCost),
          name(data.name),
          period(data.period),
          changePolicy(data.changePolicy),
          changePolicyTimeout(data.changePolicyTimeout)
    {}
    TariffConfOpt& operator=(const TariffConf & tc) noexcept
    {
        fee         = tc.fee;
        free        = tc.free;
        traffType   = tc.traffType;
        passiveCost = tc.passiveCost;
        name        = tc.name;
        period      = tc.period;
        changePolicy = tc.changePolicy;
        changePolicyTimeout = tc.changePolicyTimeout;
        return *this;
    }

    TariffConfOpt(const TariffConfOpt&) = default;
    TariffConfOpt& operator=(const TariffConfOpt&) = default;
    TariffConfOpt(TariffConfOpt&&) = default;
    TariffConfOpt& operator=(TariffConfOpt&&) = default;

    TariffConf get(const TariffConf& defaultValue) const noexcept
    {
        TariffConf res;
        res.fee = fee.value_or(defaultValue.fee);
        res.free = free.value_or(defaultValue.free);
        res.traffType = traffType.value_or(defaultValue.traffType);
        res.passiveCost = passiveCost.value_or(defaultValue.passiveCost);
        res.name = name.value_or(defaultValue.name);
        res.period = period.value_or(defaultValue.period);
        res.changePolicy = changePolicy.value_or(defaultValue.changePolicy);
        res.changePolicyTimeout = changePolicyTimeout.value_or(defaultValue.changePolicyTimeout);
        return res;
    }

    std::optional<double>             fee;
    std::optional<double>             free;
    std::optional<Tariff::TraffType>  traffType;
    std::optional<double>             passiveCost;
    std::optional<std::string>        name;
    std::optional<Tariff::Period>     period;
    std::optional<Tariff::ChangePolicy> changePolicy;
    std::optional<time_t>             changePolicyTimeout;
};
//-----------------------------------------------------------------------------
struct TariffData
{
    TariffConf                tariffConf;
    std::vector<DirPriceData> dirPrice;

    TariffData() noexcept
        : dirPrice(DIR_NUM)
    {}

    explicit TariffData(const std::string& name) noexcept
        : tariffConf(name),
          dirPrice(DIR_NUM)
    {}

    TariffData(const TariffData&) = default;
    TariffData& operator=(const TariffData&) = default;
    TariffData(TariffData&&) = default;
    TariffData& operator=(TariffData&&) = default;
};
//-----------------------------------------------------------------------------
struct TariffDataOpt
{
    TariffConfOpt                tariffConf;
    std::vector<DirPriceDataOpt> dirPrice;

    TariffDataOpt()
        : dirPrice(DIR_NUM)
    {}

    TariffDataOpt(const TariffData& data) noexcept
        : tariffConf(data.tariffConf),
          dirPrice(DIR_NUM)
    {
        for (size_t i = 0; i < DIR_NUM; ++i)
            dirPrice[i] = data.dirPrice[i];
    }

    TariffDataOpt& operator=(const TariffData& td) noexcept
    {
        tariffConf = td.tariffConf;
        for (size_t i = 0; i < DIR_NUM; ++i)
            dirPrice[i] = td.dirPrice[i];
        return *this;
    }

    TariffDataOpt(const TariffDataOpt&) = default;
    TariffDataOpt& operator=(const TariffDataOpt&) = default;
    TariffDataOpt(TariffDataOpt&&) = default;
    TariffDataOpt& operator=(TariffDataOpt&&) = default;

    TariffData get(const TariffData& defaultValue) const noexcept
    {
        TariffData res;
        res.tariffConf = tariffConf.get(defaultValue.tariffConf);
        for (size_t i = 0; i < DIR_NUM; ++i)
            res.dirPrice[i] = dirPrice[i].get(defaultValue.dirPrice[i]);
        return res;
    }
};
//-----------------------------------------------------------------------------
}
