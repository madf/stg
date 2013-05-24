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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

class SETTINGS {
public:
    virtual ~SETTINGS() {}
    virtual const            std::string & GetDirName(size_t num) const = 0;
    virtual const            std::string & GetScriptsDir() const = 0;
    virtual unsigned                       GetDetailStatWritePeriod() const = 0;
    virtual unsigned                       GetStatWritePeriod() const = 0;
    virtual unsigned                       GetDayFee() const = 0;
    virtual bool                           GetFullFee() const = 0;
    virtual unsigned                       GetDayResetTraff() const = 0;
    virtual bool                           GetSpreadFee() const = 0;
    virtual bool                           GetFreeMbAllowInet() const = 0;
    virtual bool                           GetDayFeeIsLastDay() const = 0;
    virtual bool                           GetWriteFreeMbTraffCost() const = 0;
    virtual bool                           GetShowFeeInCash() const = 0;
    virtual unsigned                       GetMessageTimeout() const = 0;
    virtual unsigned                       GetFeeChargeType() const = 0;
    virtual bool                           GetReconnectOnTariffChange() const = 0;
    virtual const            std::string & GetMonitorDir() const = 0;
    virtual bool                           GetMonitoring() const = 0;
    virtual const std::vector<std::string> & GetScriptParams() const = 0;
};
//-----------------------------------------------------------------------------

#endif
