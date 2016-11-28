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

/*
 $Revision: 1.9 $
 $Date: 2010/10/07 17:53:39 $
 $Author: faust $
 */

#ifndef TARIFF_IMPL_H
#define TARIFF_IMPL_H

#include <ctime>

#include <string>
#include <list>

#include "stg/tariff.h"
#include "stg/os_int.h"
#include "stg/tariff_conf.h"

#define TARIFF_DAY     0
#define TARIFF_NIGHT   1

class TARIFF_IMPL : public TARIFF {
public:
    TARIFF_IMPL()
        : TARIFF(),
          tariffData()
    {}
    TARIFF_IMPL(const std::string & name)
        : TARIFF(),
          tariffData(name)
    {}
    TARIFF_IMPL(const TARIFF_DATA & td)
        : TARIFF(),
          tariffData(td)
    {}
    TARIFF_IMPL(const TARIFF_IMPL & t)
        : TARIFF(),
          tariffData(t.tariffData)
    {}
    virtual ~TARIFF_IMPL() {}

    double  GetPriceWithTraffType(uint64_t up,
                                  uint64_t down,
                                  int dir,
                                  time_t t) const;
    double  GetFreeMb() const { return tariffData.tariffConf.free; }
    double  GetPassiveCost() const { return tariffData.tariffConf.passiveCost; }
    double  GetFee() const { return tariffData.tariffConf.fee; }
    double  GetFree() const { return tariffData.tariffConf.free; }
    PERIOD  GetPeriod() const { return tariffData.tariffConf.period; }
    CHANGE_POLICY GetChangePolicy() const { return tariffData.tariffConf.changePolicy; }
    time_t GetChangePolicyTimeout() const { return tariffData.tariffConf.changePolicyTimeout; }

    void    Print() const;

    const   std::string & GetName() const { return tariffData.tariffConf.name; }
    void    SetName(const std::string & name) { tariffData.tariffConf.name = name; }

    int     GetTraffType() const { return tariffData.tariffConf.traffType; }
    int64_t GetTraffByType(uint64_t up, uint64_t down) const;
    int     GetThreshold(int dir) const;
    const TARIFF_DATA & GetTariffData() const { return tariffData; }

    TARIFF_IMPL & operator=(const TARIFF_DATA & td);
    TARIFF_IMPL & operator=(const TARIFF_IMPL & t);
    bool     operator==(const TARIFF_IMPL & rhs) const { return GetName() == rhs.GetName(); }
    bool     operator!=(const TARIFF_IMPL & rhs) const { return GetName() != rhs.GetName(); }
    std::string TariffChangeIsAllowed(const TARIFF & to, time_t currentTime) const;

private:
    TARIFF_DATA     tariffData;

    double  GetPriceWithoutFreeMb(int dir, int64_t mb, time_t t) const;
    int     Interval(int dir, time_t t) const;
};

#endif
