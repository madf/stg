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


#ifndef SETTINGS_H
#define SETTINGS_H

#include <vector>

#include "common.h"
#include "base_settings.h"

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
class STG_LOGGER;
class DOTCONFDocumentNode;
//-----------------------------------------------------------------------------
class SETTINGS
{
public:
    SETTINGS();
    SETTINGS(const std::string &);
    SETTINGS(const SETTINGS &);
    virtual ~SETTINGS();
    int Reload() { return ReadSettings(); };
    int ReadSettings();

    string GetStrError() const { return strError; };

    int             GetExecMsgKey() const { return stgExecMsgKey; };
    size_t          GetExecutersNum() const { return executersNum; };
    const string &  GetDirName(int num) const { return dirName[num]; };
    const string &  GetConfDir() const { return confDir; };
    const string &  GetScriptDir() const { return scriptDir; };
    const string &  GetRulesFileName() const { return rules; };
    const string &  GetLogFileName() const { return logFile; };
    const string &  GetPIDFileName() const { return pidFile; };
    unsigned        GetDetailStatWritePeriod() const 
        { return detailStatWritePeriod; };
    unsigned        GetStatWritePeriod() const { return statWritePeriod * 60; };
    unsigned        GetDayFee() const { return dayFee; };
    bool            GetFullFee() const { return fullFee; };
    unsigned        GetDayResetTraff() const { return dayResetTraff; };
    bool            GetSpreadFee() const { return spreadFee; };
    bool            GetFreeMbAllowInet() const { return freeMbAllowInet; };
    bool            GetDayFeeIsLastDay() const { return dayFeeIsLastDay; };
    bool            GetWriteFreeMbTraffCost() const
        { return writeFreeMbTraffCost; };
    bool            GetShowFeeInCash() const { return showFeeInCash; };
    const string  & GetMonitorDir() const { return monitorDir; };
    bool            GetMonitoring() const { return monitoring; };
    unsigned        GetMessageTimeout() const { return messageTimeout * 3600 * 24; };

    const string &  GetModulesPath() const { return modulesPath; };
    const MODULE_SETTINGS         & GetStoreModuleSettings() const
        { return storeModuleSettings; };
    const vector<MODULE_SETTINGS> & GetModulesSettings() const
        { return modulesSettings; };

private:

    int ParseInt(const string & value, int * val);
    int ParseUnsigned(const string & value, unsigned * val);
    int ParseIntInRange(const string & value, int min, int max, int * val);
    int ParseUnsignedInRange(const string & value, unsigned min, unsigned max, unsigned * val);
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
    unsigned    detailStatWritePeriod;
    unsigned    statWritePeriod;
    int         stgExecMsgKey;
    size_t      executersNum;
    bool        fullFee;
    unsigned    dayFee;        // день снятия абонплаты
    unsigned    dayResetTraff; // Начало учетного периода: день обнуления трафика и смены тарифа
    bool        spreadFee;
    bool        freeMbAllowInet;
    bool        dayFeeIsLastDay; // АП снимается в конце месяца (true) или в начале (false)
    bool        writeFreeMbTraffCost; // Писать в детальную статистику стоимость трафика, если еще есть предоплаченный трафик
    bool        showFeeInCash; // Показывать пользователю АП не счету и позволять ее использовать
    unsigned    messageTimeout; // Время жизни неотправленного сообщения в секундах

    vector<MODULE_SETTINGS> modulesSettings;
    MODULE_SETTINGS         storeModuleSettings;

    STG_LOGGER & logger;
};
//-----------------------------------------------------------------------------
#endif
