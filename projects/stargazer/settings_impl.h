 /*
 $Revision: 1.27 $
 $Date: 2010/08/19 13:42:30 $
 $Author: faust $
 */

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
 *    Date: 27.10.2002
 */

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

 /*
 $Revision: 1.27 $
 $Date: 2010/08/19 13:42:30 $
 */


#ifndef SETTINGS_IMPL_H
#define SETTINGS_IMPL_H

#include <string>
#include <vector>

#include "stg/settings.h"
#include "stg/common.h"
#include "stg/module_settings.h"

//-----------------------------------------------------------------------------
enum DETAIL_STAT_PERIOD {
dsPeriod_1,
dsPeriod_1_2,
dsPeriod_1_4,
dsPeriod_1_6
};
//-----------------------------------------------------------------------------
class STG_LOGGER;
class DOTCONFDocumentNode;
//-----------------------------------------------------------------------------
class SETTINGS_IMPL : public SETTINGS {
public:
    SETTINGS_IMPL(const std::string &);
    SETTINGS_IMPL(const SETTINGS_IMPL &);
    virtual ~SETTINGS_IMPL() {}
    SETTINGS_IMPL & operator=(const SETTINGS_IMPL &);

    int Reload() { return ReadSettings(); }
    int ReadSettings();

    std::string GetStrError() const { return strError; }

    int                 GetExecMsgKey() const { return stgExecMsgKey; }
    unsigned            GetExecutersNum() const { return executersNum; }
    const std::string & GetDirName(size_t num) const { return dirName[num]; }
    const std::string & GetConfDir() const { return confDir; }
    const std::string & GetScriptsDir() const { return scriptsDir; }
    const std::string & GetRulesFileName() const { return rules; }
    const std::string & GetLogFileName() const { return logFile; }
    const std::string & GetPIDFileName() const { return pidFile; }
    unsigned            GetDetailStatWritePeriod() const 
        { return detailStatWritePeriod; }
    unsigned            GetStatWritePeriod() const { return statWritePeriod * 60; }
    unsigned            GetDayFee() const { return dayFee; }
    bool                GetFullFee() const { return fullFee; }
    unsigned            GetDayResetTraff() const { return dayResetTraff; }
    bool                GetSpreadFee() const { return spreadFee; }
    bool                GetFreeMbAllowInet() const { return freeMbAllowInet; }
    bool                GetDayFeeIsLastDay() const { return dayFeeIsLastDay; }
    bool                GetWriteFreeMbTraffCost() const
        { return writeFreeMbTraffCost; }
    bool                GetShowFeeInCash() const { return showFeeInCash; }
    const std::string & GetMonitorDir() const { return monitorDir; }
    bool                GetMonitoring() const { return monitoring; }
    unsigned            GetMessageTimeout() const { return messageTimeout * 3600 * 24; }
    unsigned            GetFeeChargeType() const { return feeChargeType; }
    bool                GetReconnectOnTariffChange() const { return reconnectOnTariffChange; }
    bool                GetDisableSessionLog() const { return disableSessionLog; }
    const std::vector<std::string> & GetFilterParamsLog() const { return filterParamsLog; }

    const std::string & GetModulesPath() const { return modulesPath; }
    const MODULE_SETTINGS & GetStoreModuleSettings() const
        { return storeModuleSettings; }
    const std::vector<MODULE_SETTINGS> & GetModulesSettings() const
        { return modulesSettings; }
    const std::vector<std::string> & GetScriptParams() const { return scriptParams; }

private:

    int ParseDetailStatWritePeriod(const std::string & str);
    int ParseModuleSettings(const DOTCONFDocumentNode * dirNameNode, std::vector<PARAM_VALUE> * params);

    static void ErrorCallback(void * data, const char * buf);

    std::string strError;

    //////////settings
    std::string modulesPath;
    std::vector<std::string> dirName;
    std::string confDir;
    std::string	scriptsDir;
    std::string rules;
    std::string logFile;
    std::string pidFile;
    std::string monitorDir;
    std::vector<std::string> scriptParams;
    bool        monitoring;
    unsigned    detailStatWritePeriod;
    unsigned    statWritePeriod;
    int         stgExecMsgKey;
    unsigned    executersNum;
    bool        fullFee;
    unsigned    dayFee;
    unsigned    dayResetTraff;
    bool        spreadFee;
    bool        freeMbAllowInet;
    bool        dayFeeIsLastDay;
    bool        writeFreeMbTraffCost;
    bool        showFeeInCash;
    unsigned    messageTimeout;
    unsigned    feeChargeType;
    bool        reconnectOnTariffChange;
    bool        disableSessionLog;
    std::vector<std::string> filterParamsLog;

    std::vector<MODULE_SETTINGS> modulesSettings;
    MODULE_SETTINGS storeModuleSettings;
    STG_LOGGER & logger;
};
//-----------------------------------------------------------------------------

#endif
