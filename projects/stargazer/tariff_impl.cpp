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
 $Revision: 1.11 $
 $Date: 2010/10/07 16:57:21 $
 $Author: faust $
 */

#include <ctime>
#include <algorithm> // std::max

#include "stg/common.h"
#include "tariff_impl.h"
#include "stg_timer.h"

//-----------------------------------------------------------------------------
TARIFF_IMPL & TARIFF_IMPL::operator=(const TARIFF_DATA & td)
{
tariffData = td;
return *this;
}
//-----------------------------------------------------------------------------
TARIFF_IMPL & TARIFF_IMPL::operator=(const TARIFF_IMPL & t)
{
tariffData = t.tariffData;
return *this;
}
//-----------------------------------------------------------------------------
double TARIFF_IMPL::GetPriceWithTraffType(uint64_t up,
                                     uint64_t down,
                                     int dir,
                                     time_t t) const
{
return GetPriceWithoutFreeMb(dir, GetTraffByType(up, down) / (1024 * 1024), t);
}
//-----------------------------------------------------------------------------
int64_t TARIFF_IMPL::GetTraffByType(uint64_t up, uint64_t down) const
{
switch (tariffData.tariffConf.traffType)
    {
    case TRAFF_UP:
        return up;

    case TRAFF_DOWN:
        return down;

    case TRAFF_MAX:
        return std::max(up, down);

    default:  //TRAFF_UP_DOWN:
        return up + down;
    }
}
//-----------------------------------------------------------------------------
int TARIFF_IMPL::GetThreshold(int dir) const
{
    return tariffData.dirPrice[dir].threshold;
}
//-----------------------------------------------------------------------------
void TARIFF_IMPL::Print() const
{
printfd(__FILE__, "Traiff name: %s\n", tariffData.tariffConf.name.c_str());
}
//-----------------------------------------------------------------------------
int TARIFF_IMPL::Interval(int dir, time_t t) const
{
// Start of the day (and end of the night) in sec from 00:00:00
int s1 = tariffData.dirPrice[dir].hDay * 3600 +
         tariffData.dirPrice[dir].mDay * 60;
// Start of the night (and end of the day) in sec from 00:00:00
int s2 = tariffData.dirPrice[dir].hNight * 3600 +
         tariffData.dirPrice[dir].mNight * 60;

struct tm * lt;

lt = localtime(&t);

// Position of time t in sec from 00:00:00
// Ignoring seconds due to minute precision
int lts = lt->tm_hour * 3600 + lt->tm_min * 60;

if (s1 < s2)
    {
    // Normal situation (00:00:00 is a night)
    if (lts > s1 && lts < s2)
        return TARIFF_DAY;
    else
        return TARIFF_NIGHT;
    }
else
    {
    // Not so common but possible situation (00:00:00 is a day)
    if (lts < s1 && lts > s2)
        return TARIFF_NIGHT;
    else
        return TARIFF_DAY;
    }
}
//-----------------------------------------------------------------------------
double TARIFF_IMPL::GetPriceWithoutFreeMb(int dir, int mb, time_t t) const
{
int interval = Interval(dir, t);

/*
 * 0011 - NB
 * *01* - NA
 * 0**1 - DB
 * **** - DA
 */

bool nd = tariffData.dirPrice[dir].noDiscount;
bool sp = tariffData.dirPrice[dir].singlePrice;
bool th = (interval == TARIFF_NIGHT);
bool tr = (mb > tariffData.dirPrice[dir].threshold);

if (!nd && !sp && th && tr)
    return tariffData.dirPrice[dir].priceNightB;
else if (!nd && tr)
    return tariffData.dirPrice[dir].priceDayB;
else if (!sp && th)
    return tariffData.dirPrice[dir].priceNightA;
else
    return tariffData.dirPrice[dir].priceDayA;

/*if (tariffData.dirPrice[dir].noDiscount && tariffData.dirPrice[dir].singlePrice)
    {
    return tariffData.dirPrice[dir].priceDayA;
    }
else
    {
    if (tariffData.dirPrice[dir].noDiscount)
        {
        // Without threshold
        if (interval == TARIFF_DAY)
            return tariffData.dirPrice[dir].priceDayA;
        else
            return tariffData.dirPrice[dir].priceNightA;
        }

    if (tariffData.dirPrice[dir].singlePrice)
        {
        // Without day/night
        if (mb < tariffData.dirPrice[dir].threshold)
            return tariffData.dirPrice[dir].priceDayA;
        else
            return tariffData.dirPrice[dir].priceDayB;
        }

    if (mb < tariffData.dirPrice[dir].threshold)
        {
        if (interval == TARIFF_DAY)
            return tariffData.dirPrice[dir].priceDayA;
        else
            return tariffData.dirPrice[dir].priceNightA;
        }
    else
        {
        if (interval == TARIFF_DAY)
            return tariffData.dirPrice[dir].priceDayB;
        else
            return tariffData.dirPrice[dir].priceNightB;
        }
    }*/
}
//-----------------------------------------------------------------------------
