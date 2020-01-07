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

/*
 *  Vairous utility methods
 *
 *  $Revision: 1.8 $
 *  $Date: 2010/03/04 12:20:32 $
 *
 */

#include <cstdio>

#include "firebird_store.h"
#include "stg/ibpp.h"

//-----------------------------------------------------------------------------
time_t ts2time_t(const IBPP::Timestamp & ts)
{
    char buf[32];
    int year, month, day, hour, min, sec;
    struct tm time_tm;

    memset(&time_tm, 0, sizeof(time_tm));
    ts.GetDate(year, month, day);
    ts.GetTime(hour, min, sec);
    if (year < 1990)
        return 0;
    sprintf(buf, "%d-%d-%d %d:%d:%d", year, month, day, hour, min, sec);
    stg_strptime(buf, "%Y-%m-%d %H:%M:%S", &time_tm);

    return mktime(&time_tm);
}
//-----------------------------------------------------------------------------
void time_t2ts(time_t t, IBPP::Timestamp * ts)
{
    struct tm res;

    localtime_r(&t, &res); // Reenterable

    *ts = IBPP::Timestamp(res.tm_year + 1900, res.tm_mon + 1, res.tm_mday, res.tm_hour, res.tm_min, res.tm_sec);
}
//-----------------------------------------------------------------------------
void ym2date(int year, int month, IBPP::Date * date)
{
    date->SetDate(year + 1900, month + 1, 1);
    date->EndOfMonth();
}
//-----------------------------------------------------------------------------
