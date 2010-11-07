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


#ifndef settingsh_h
#define settingsh_h 1

#include <sys/types.h>
#include <vector>
#include <dotconfpp.h>

#include "common.h"
#include "base_settings.h"
#include "stg_logger.h"

using namespace std;

//-----------------------------------------------------------------------------
enum DETAIL_STAT_PERIOD
{
dsPeriod_1,
dsPeriod_1_2,
dsPeriod_1_4,
dsPeriod_1_6,
};
//-----------------------------------------------------------------------------
class SETTINGS
{
public:
    SETTINGS();
    SETTINGS(const std::string &);
    SETTINGS(const SETTINGS &);
    virtual ~SETTINGS();
    int Reload();
    int ReadSettings();

    string GetStrError() const;

    int             GetExecMsgKey() const { return stgExecMsgKey; };
    int             GetExecutersNum() const { return executersNum; };
    //int             GetExecutersWaitTimeout() const;
    const string &  GetDirName(int num) const { return dirName[num]; };
    const string &  GetConfDir() const { return confDir; };
    const string &  GetScriptDir() const { return scriptDir; };
    const string &  GetRulesFileName() const { return rules; };
    const string &  GetLogFileName() const { return logFile; };
    const string &  GetPIDFileName() const { return pidFile; };
    int             GetDetailStatWritePeriod() const 
        { return detailStatWritePeriod; };
    int             GetStatWritePeriod() const { return statWritePeriod * 60; };
    int             GetDayFee() const { return dayFee; };
    bool            GetFullFee() const { return fullFee; };
    int             GetDayResetTraff() const { return dayResetTraff; };
    bool            GetSpreadFee() const { return spreadFee; };
    bool            GetFreeMbAllowInet() const { return freeMbAllowInet; };
    bool            GetDayFeeIsLastDay() const { return dayFeeIsLastDay; };
    bool            GetWriteFreeMbTraffCost() const
        { return writeFreeMbTraffCost; };
    bool            GetShowFeeInCash() const { return showFeeInCash; };
    const string  & GetMonitorDir() const { return monitorDir; };
    bool            GetMonitoring() const { return monitoring; };

    const string &  GetModulesPath() const { return modulesPath; };
    const MODULE_SETTINGS         & GetStoreModuleSettings() const;
    const vector<MODULE_SETTINGS> & GetModulesSettings() const;

private:

    int ParseInt(const string & value, int * val);
    int ParseIntInRange(const string & value, int min, int max, int * val);
    int ParseYesNo(const string & value, bool * val);
    int ParseDetailStatWritePeriod(const string & detailStatPeriodStr);

    int ParseModuleSettings(const DOTCONFDocumentNode * dirNameNode, vector<PARAM_VALUE> * params);

    static void ErrorCallback(void * data, const char * buf);

    string      strError;
    //////////settings
    string      modulesPath;
    string      dirName[DIR_NUM];
    string      confDir;
    string	scriptDir;
    string      rules;
    string      logFile;
    string      pidFile;
    string      monitorDir;
    bool        monitoring;
    int         detailStatWritePeriod;
    int         statWritePeriod;
    int         stgExecMsgKey;
    int         executersNum;
    //int         executersWaitTimeout;
    bool        fullFee;
    int         dayFee;        // день снятия абонплаты
    int         dayResetTraff; // Начало учетного периода: день обнуления трафика и смены тарифа
    bool        spreadFee;
    bool        freeMbAllowInet;
    bool        dayFeeIsLastDay; // АП снимается в конце месяца (true) или в начале (false)
    bool        writeFreeMbTraffCost; // Писать в детальную статистику стоимость трафика, если еще есть предоплаченный трафик
    bool        showFeeInCash; // Показывать пользователю АП не счету и позволять ее использовать

    vector<MODULE_SETTINGS> modulesSettings;
    MODULE_SETTINGS         storeModuleSettings;

    STG_LOGGER & logger;
};
//-----------------------------------------------------------------------------
#endif

