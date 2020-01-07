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

#include "settings_impl.h"

#include "stg/logger.h"
#include "stg/dotconfpp.h"
#include "stg/common.h"
#include "stg/const.h"

#include <stdexcept>
#include <cstring>
#include <cerrno>

namespace
{

struct Error : public std::runtime_error
{
    Error(const std::string& message) : runtime_error(message) {}
};

std::vector<std::string> toValues(const DOTCONFDocumentNode& node)
{
    std::vector<std::string> values;

    size_t i = 0;
    const char* value = NULL;
    while ((value = node.getValue(i++)) != NULL)
        values.push_back(value);

    return values;
}

std::vector<PARAM_VALUE> toPVS(const DOTCONFDocumentNode& node)
{
    std::vector<PARAM_VALUE> pvs;

    const DOTCONFDocumentNode* child = node.getChildNode();
    while (child != NULL)
        {
        if (child->getName() == NULL)
            continue;

        if (child->getChildNode() == NULL)
            pvs.push_back(PARAM_VALUE(child->getName(), toValues(*child)));
        else
            pvs.push_back(PARAM_VALUE(child->getName(), toValues(*child), toPVS(*child)));

        child = child->getNextNode();
        }

    return pvs;
}

unsigned toPeriod(const char* value)
{
    if (value == NULL)
        throw Error("No detail stat period value.");

    std::string period(value);
    if (period == "1")
        return dsPeriod_1;
    else if (period == "1/2")
        return dsPeriod_1_2;
    else if (period == "1/4")
        return dsPeriod_1_4;
    else if (period == "1/6")
        return dsPeriod_1_6;

    throw Error("Invalid detail stat period value: '" + period + "'. Should be one of '1', '1/2', '1/4' or '1/6'.");
}

}

//-----------------------------------------------------------------------------
SETTINGS_IMPL::SETTINGS_IMPL(const std::string & cd)
    : modulesPath("/usr/lib/stg"),
      dirName(DIR_NUM),
      confDir(cd.empty() ? "/etc/stargazer" : cd),
      scriptsDir(confDir),
      rules(confDir + "/rules"),
      logFile("/var/log/stargazer.log"),
      pidFile("/var/run/stargazer.pid"),
      monitorDir("/var/stargazer/monitoring"),
      monitoring(false),
      detailStatWritePeriod(dsPeriod_1_6),
      statWritePeriod(10),
      stgExecMsgKey(5555),
      executersNum(1),
      fullFee(false),
      dayFee(0),
      dayResetTraff(0),
      spreadFee(false),
      freeMbAllowInet(false),
      dayFeeIsLastDay(false),
      stopOnError(true),
      writeFreeMbTraffCost(false),
      showFeeInCash(true),
      messageTimeout(0),
      feeChargeType(0),
      reconnectOnTariffChange(false),
      disableSessionLog(false),
      logger(GetStgLogger())
{
    filterParamsLog.push_back("*");
}
//-----------------------------------------------------------------------------
SETTINGS_IMPL::SETTINGS_IMPL(const SETTINGS_IMPL & rval)
    : modulesPath(rval.modulesPath),
      dirName(rval.dirName),
      confDir(rval.confDir),
      scriptsDir(rval.scriptsDir),
      rules(rval.rules),
      logFile(rval.logFile),
      pidFile(rval.pidFile),
      monitorDir(rval.monitorDir),
      monitoring(rval.monitoring),
      detailStatWritePeriod(rval.detailStatWritePeriod),
      statWritePeriod(rval.statWritePeriod),
      stgExecMsgKey(rval.stgExecMsgKey),
      executersNum(rval.executersNum),
      fullFee(rval.fullFee),
      dayFee(rval.dayFee),
      dayResetTraff(rval.dayResetTraff),
      spreadFee(rval.spreadFee),
      freeMbAllowInet(rval.freeMbAllowInet),
      dayFeeIsLastDay(rval.dayFeeIsLastDay),
      stopOnError(rval.stopOnError),
      writeFreeMbTraffCost(rval.writeFreeMbTraffCost),
      showFeeInCash(rval.showFeeInCash),
      messageTimeout(rval.messageTimeout),
      feeChargeType(rval.feeChargeType),
      reconnectOnTariffChange(rval.reconnectOnTariffChange),
      disableSessionLog(rval.disableSessionLog),
      filterParamsLog(rval.filterParamsLog),
      modulesSettings(rval.modulesSettings),
      storeModuleSettings(rval.storeModuleSettings),
      logger(GetStgLogger())
{
}
//-----------------------------------------------------------------------------
SETTINGS_IMPL & SETTINGS_IMPL::operator=(const SETTINGS_IMPL & rhs)
{
    modulesPath = rhs.modulesPath;
    dirName = rhs.dirName;
    confDir = rhs.confDir;
    scriptsDir = rhs.scriptsDir;
    rules = rhs.rules;
    logFile = rhs.logFile;
    pidFile = rhs.pidFile;
    monitorDir = rhs.monitorDir;
    scriptParams = rhs.scriptParams;
    monitoring = rhs.monitoring;
    detailStatWritePeriod = rhs.detailStatWritePeriod;
    statWritePeriod = rhs.statWritePeriod;
    stgExecMsgKey = rhs.stgExecMsgKey;
    executersNum = rhs.executersNum;
    fullFee = rhs.fullFee;
    dayFee = rhs.dayFee;
    dayResetTraff = rhs.dayResetTraff;
    spreadFee = rhs.spreadFee;
    freeMbAllowInet = rhs.freeMbAllowInet;
    dayFeeIsLastDay = rhs.dayFeeIsLastDay;
    stopOnError = rhs.stopOnError;
    writeFreeMbTraffCost = rhs.writeFreeMbTraffCost;
    showFeeInCash = rhs.showFeeInCash;
    messageTimeout = rhs.messageTimeout;
    feeChargeType = rhs.feeChargeType;
    reconnectOnTariffChange = rhs.reconnectOnTariffChange;
    disableSessionLog = rhs.disableSessionLog;
    filterParamsLog = rhs.filterParamsLog;

    modulesSettings = rhs.modulesSettings;
    storeModuleSettings = rhs.storeModuleSettings;
    return *this;
}
//-----------------------------------------------------------------------------
void SETTINGS_IMPL::ErrorCallback(void * data, const char * buf)
{
    printfd(__FILE__, "SETTINGS_IMPL::ErrorCallback() - %s\n", buf);
    SETTINGS_IMPL * settings = static_cast<SETTINGS_IMPL *>(data);
    settings->logger("%s", buf);
}
//-----------------------------------------------------------------------------
int SETTINGS_IMPL::ReadSettings()
{
const char * requiredOptions[] = {
    "ModulesPath",
    "Modules",
    "StoreModule",
    "Rules",
    "LogFile",
    "DetailStatWritePeriod",
    "DayFee",
    "DayResetTraff",
    "SpreadFee",
    "FreeMbAllowInet",
    "DayFeeIsLastDay",
    "WriteFreeMbTraffCost",
    NULL
    };
int storeModulesCount = 0;
modulesSettings.clear();

DOTCONFDocument conf(DOTCONFDocument::CASEINSENSITIVE);
conf.setErrorCallback(SETTINGS_IMPL::ErrorCallback, this);
conf.setRequiredOptionNames(requiredOptions);
std::string confFile = confDir + "/stargazer.conf";

if(conf.setContent(confFile.c_str()) != 0)
    {
    strError = "Cannot read file " + confFile;
    return -1;
    }

const DOTCONFDocumentNode * node = conf.getFirstNode();

while (node)
    {
    if (strcasecmp(node->getName(), "ScriptDir") == 0)
        {
        scriptsDir = node->getValue(0);
        }

    if (strcasecmp(node->getName(), "LogFile") == 0)
        {
        logFile = node->getValue(0);
        }

    if (strcasecmp(node->getName(), "PIDFile") == 0)
        {
        pidFile = node->getValue(0);
        }

    if (strcasecmp(node->getName(), "ModulesPath") == 0)
        {
        modulesPath = node->getValue(0);
        }

    if (strcasecmp(node->getName(), "Rules") == 0)
        {
        rules = node->getValue(0);
        }

    if (strcasecmp(node->getName(), "DetailStatWritePeriod") == 0)
        {
        try
        {
            detailStatWritePeriod = toPeriod(node->getValue(0));
        }
        catch (const Error& error)
        {
            strError = error.what();
            return -1;
        }
        }

    if (strcasecmp(node->getName(), "StatWritePeriod") == 0)
        {
        if (ParseUnsignedInRange(node->getValue(0), 1, 1440, &statWritePeriod) != 0)
            {
            strError = "Incorrect StatWritePeriod value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "ExecMsgKey") == 0)
        {
        if (ParseInt(node->getValue(0), &stgExecMsgKey) != 0)
            {
            strError = "Incorrect ExecMsgKey value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "ExecutersNum") == 0)
        {
        if (ParseUnsignedInRange(node->getValue(0), 1, 1024, &executersNum) != 0)
            {
            strError = "Incorrect ExecutersNum value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "DayFee") == 0)
        {
        if (ParseUnsignedInRange(node->getValue(0), 0, 31, &dayFee) != 0)
            {
            strError = "Incorrect DayFee value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "FullFee") == 0)
        {
        if (ParseYesNo(node->getValue(0), &fullFee) != 0)
            {
            strError = "Incorrect FullFee value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "DayResetTraff") == 0)
        {
        if (ParseUnsignedInRange(node->getValue(0), 0, 31, &dayResetTraff) != 0)
            {
            strError = "Incorrect DayResetTraff value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "SpreadFee") == 0)
        {
        if (ParseYesNo(node->getValue(0), &spreadFee) != 0)
            {
            strError = "Incorrect SpreadFee value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "FreeMbAllowInet") == 0)
        {
        if (ParseYesNo(node->getValue(0), &freeMbAllowInet) != 0)
            {
            strError = "Incorrect FreeMbAllowInet value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "DayFeeIsLastDay") == 0)
        {
        if (ParseYesNo(node->getValue(0), &dayFeeIsLastDay) != 0)
            {
            strError = "Incorrect DayFeeIsLastDay value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "StopOnError") == 0)
        {
        if (ParseYesNo(node->getValue(0), &stopOnError) != 0)
            {
            strError = "Incorrect StopOnError value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "WriteFreeMbTraffCost") == 0)
        {
        if (ParseYesNo(node->getValue(0), &writeFreeMbTraffCost) != 0)
            {
            strError = "Incorrect WriteFreeMbTraffCost value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "ShowFeeInCash") == 0)
        {
        if (ParseYesNo(node->getValue(0), &showFeeInCash) != 0)
            {
            strError = "Incorrect ShowFeeInCash value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "MonitorDir") == 0)
        {
        monitorDir = node->getValue(0);
        struct stat stat;
        monitoring = false;

        if (!lstat(monitorDir.c_str(), &stat) && S_ISDIR(stat.st_mode))
            {
            monitoring = true;
            }
        }

    if (strcasecmp(node->getName(), "MessageTimeout") == 0)
        {
        if (ParseUnsigned(node->getValue(0), &messageTimeout) != 0)
            {
            strError = "Incorrect MessageTimeout value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "FeeChargeType") == 0)
        {
        if (ParseUnsignedInRange(node->getValue(0), 0, 3, &feeChargeType) != 0)
            {
            strError = "Incorrect FeeChargeType value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "ReconnectOnTariffChange") == 0)
        {
        if (ParseYesNo(node->getValue(0), &reconnectOnTariffChange) != 0)
            {
            strError = "Incorrect ReconnectOnTariffChange value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "DisableSessionLog") == 0)
        {
        if (ParseYesNo(node->getValue(0), &disableSessionLog) != 0)
            {
            strError = "Incorrect DisableSessionLog value: \'" + std::string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "FilterParamsLog") == 0)
        {
        filterParamsLog.clear();
        for (int i = 0; node->getValue(i) != NULL; ++i)
            filterParamsLog.push_back(node->getValue(i));
        }

    if (strcasecmp(node->getName(), "DirNames") == 0)
        {
        const DOTCONFDocumentNode * child = node->getChildNode();
        if (child)
            {
            const DOTCONFDocumentNode * dirNameNode;
            dirName.reserve(DIR_NUM);
            for (int i = 0; i < DIR_NUM; i++)
                {
                char strDirName[12];
                sprintf(strDirName, "DirName%d", i);
                dirNameNode = conf.findNode(strDirName, node);
                if (dirNameNode && dirNameNode->getValue(0))
                    {
                    dirName[i] = dirNameNode->getValue(0);
                    }
                }
            }
        }

    if (strcasecmp(node->getName(), "StoreModule") == 0)
        {
        if (node->getValue(1))
            {
            strError = "Unexpected \'" + std::string(node->getValue(1)) + "\'.";
            return -1;
            }

        if (storeModulesCount)
            {
            strError = "Should be only one StoreModule.";
            return -1;
            }
        storeModulesCount++;

        if (node->getValue(0) == NULL)
            {
            strError = "No module name in the StoreModule section.";
            return -1;
            }
        storeModuleSettings.moduleName = node->getValue(0);
        storeModuleSettings.moduleParams = toPVS(*node);
        }

    if (strcasecmp(node->getName(), "Modules") == 0)
        {
        if (node->getValue(0))
            {
            strError = "Unexpected \'" + std::string(node->getValue(0)) + "\'.";
            return -1;
            }
        const DOTCONFDocumentNode * child = node->getChildNode();
        while (child)
            {
            if (strcasecmp(child->getName(), "Module") != 0)
                {
                child = child->getNextNode();
                continue;
                }

            if (child->getValue(0) == NULL)
                {
                strError = "No module name in the Module section.";
                return -1;
                }

            modulesSettings.push_back(MODULE_SETTINGS(child->getValue(0), toPVS(*child)));

            child = child->getNextNode();
            }
        }

    if (strcasecmp(node->getName(), "ScriptParams") == 0)
        {
        for (int i = 0; node->getValue(i) != NULL; ++i)
            scriptParams.push_back(node->getValue(i));
        }
    node = node->getNextNode();
    }

return 0;
}
//-----------------------------------------------------------------------------
