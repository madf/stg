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
 *    Date: 07.11.2007
 */

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

#pragma once

#include "stg/tariff.h"
#include "stg/tariff_conf.h"

#include <string>

#include <ctime>
#include <cstdint>

#define TARIFF_DAY     0
#define TARIFF_NIGHT   1

namespace STG
{

class TariffImpl : public Tariff {
    public:
        explicit TariffImpl(const std::string & name)
            : tariffData(name)
        {}
        explicit TariffImpl(const TariffData & td)
            : tariffData(td)
        {}

        TariffImpl(const TariffImpl&) = default;
        TariffImpl& operator=(const TariffImpl&) = default;
        TariffImpl(TariffImpl&&) = default;
        TariffImpl& operator=(TariffImpl&&) = default;

        double  GetPriceWithTraffType(uint64_t up,
                                      uint64_t down,
                                      int dir,
                                      time_t t) const;
        double  GetFreeMb() const { return tariffData.tariffConf.free; }
        double  GetPassiveCost() const { return tariffData.tariffConf.passiveCost; }
        double  GetFee() const { return tariffData.tariffConf.fee; }
        double  GetFree() const { return tariffData.tariffConf.free; }
        Period  GetPeriod() const { return tariffData.tariffConf.period; }
        ChangePolicy GetChangePolicy() const { return tariffData.tariffConf.changePolicy; }
        time_t GetChangePolicyTimeout() const { return tariffData.tariffConf.changePolicyTimeout; }

        void    Print() const;

        const   std::string & GetName() const { return tariffData.tariffConf.name; }
        void    SetName(const std::string & name) { tariffData.tariffConf.name = name; }

        int     GetTraffType() const { return tariffData.tariffConf.traffType; }
        int64_t GetTraffByType(uint64_t up, uint64_t down) const;
        int     GetThreshold(int dir) const;
        const TariffData & GetTariffData() const { return tariffData; }

        TariffImpl & operator=(const TariffData & td);
        bool     operator==(const TariffImpl & rhs) const { return GetName() == rhs.GetName(); }
        bool     operator!=(const TariffImpl & rhs) const { return GetName() != rhs.GetName(); }
        std::string TariffChangeIsAllowed(const Tariff & to, time_t currentTime) const;

    private:
        TariffData     tariffData;

        double  GetPriceWithoutFreeMb(int dir, int64_t mb, time_t t) const;
        int     Interval(int dir, time_t t) const;
};

}
