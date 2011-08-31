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

#ifndef TARIFF_H
#define TARIFF_H

#include <ctime>

#include <string>

#include "os_int.h"
#include "tariff_conf.h"

class TARIFF {
public:
    virtual double  GetPriceWithTraffType(uint64_t up,
                                          uint64_t down,
                                          int dir,
                                          time_t t) const = 0;
    virtual double  GetFreeMb() const = 0;
    virtual double  GetPassiveCost() const = 0;
    virtual double  GetFee() const = 0;
    virtual double  GetFree() const = 0;

    virtual const   std::string & GetName() const = 0;
    virtual void    SetName(const std::string & name) = 0;

    virtual int     GetTraffType() const = 0;
    virtual int64_t GetTraffByType(uint64_t up, uint64_t down) const = 0;
    virtual int     GetThreshold(int dir) const = 0;
    virtual const TARIFF_DATA & GetTariffData() const = 0;
};

#endif
