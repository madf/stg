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
$Revision: 1.45 $
$Date: 2010/08/19 13:42:30 $
$Author: faust $
*/

#include <cstring>
#include <cerrno>
#include <string>

#include "settings_impl.h"
#include "stg_logger.h"
#include "dotconfpp.h"

using namespace std;

//-----------------------------------------------------------------------------
SETTINGS_IMPL::SETTINGS_IMPL()
    : strError(),
      modulesPath("/usr/lib/stg"),
      confDir("/etc/stargazer"),
      scriptsDir("/etc/stargazer"),
      rules("/etc/stargazer/rules"),
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
      writeFreeMbTraffCost(false),
      showFeeInCash(true),
      messageTimeout(0),
      modulesSettings(),
      storeModuleSettings(),
      logger(GetStgLogger())
{
}
//-----------------------------------------------------------------------------
SETTINGS_IMPL::SETTINGS_IMPL(const std::string & cd)
    : strError(),
      modulesPath("/usr/lib/stg"),
      dirName(DIR_NUM),
      confDir(cd),
      scriptsDir(cd),
      rules(cd + "/rules"),
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
      writeFreeMbTraffCost(false),
      showFeeInCash(true),
      messageTimeout(0),
      modulesSettings(),
      storeModuleSettings(),
      logger(GetStgLogger())
{
}
//-----------------------------------------------------------------------------
SETTINGS_IMPL::SETTINGS_IMPL(const SETTINGS_IMPL & rval)
    : strError(),
      modulesPath(rval.modulesPath),
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
      writeFreeMbTraffCost(rval.writeFreeMbTraffCost),
      showFeeInCash(rval.showFeeInCash),
      messageTimeout(rval.messageTimeout),
      modulesSettings(rval.modulesSettings),
      storeModuleSettings(rval.storeModuleSettings),
      logger(GetStgLogger())
{
}
//-----------------------------------------------------------------------------
int SETTINGS_IMPL::ParseYesNo(const string & value, bool * val)
{
if (0 == strcasecmp(value.c_str(), "yes"))
    {
    *val = true;
    return 0;
    }
if (0 == strcasecmp(value.c_str(), "no"))
    {
    *val = false;
    return 0;
    }

strError = "Incorrect value \'" + value + "\'.";
return -1;
}
//-----------------------------------------------------------------------------
int SETTINGS_IMPL::ParseInt(const string & value, int * val)
{
if (str2x<int>(value, *val))
    {
    strError = "Cannot convert \'" + value + "\' to integer.";
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int SETTINGS_IMPL::ParseUnsigned(const string & value, unsigned * val)
{
if (str2x<unsigned>(value, *val))
    {
    strError = "Cannot convert \'" + value + "\' to unsigned integer.";
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int SETTINGS_IMPL::ParseIntInRange(const string & value, int min, int max, int * val)
{
if (ParseInt(value, val) != 0)
    return -1;

if (*val < min || *val > max)
    {
    strError = "Value \'" + value + "\' out of range.";
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int SETTINGS_IMPL::ParseUnsignedInRange(const string & value, unsigned min, unsigned max, unsigned * val)
{
if (ParseUnsigned(value, val) != 0)
    return -1;

if (*val < min || *val > max)
    {
    strError = "Value \'" + value + "\' out of range.";
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int SETTINGS_IMPL::ParseModuleSettings(const DOTCONFDocumentNode * node, vector<PARAM_VALUE> * params)
{
const DOTCONFDocumentNode * childNode;
PARAM_VALUE pv;
const char * value;

pv.param = node->getName();

if (node->getValue(1))
    {
    strError = "Unexpected value \'" + string(node->getValue(1)) + "\'.";
    return -1;
    }

value = node->getValue(0);

if (!value)
    {
    strError = "Module name expected.";
    return -1;
    }

childNode = node->getChildNode();
while (childNode)
    {
    pv.param = childNode->getName();
    int i = 0;
    while ((value = childNode->getValue(i++)) != NULL)
        {
        pv.value.push_back(value);
        }
    params->push_back(pv);
    pv.value.clear();
    childNode = childNode->getNextNode();
    }

return 0;
}
//-----------------------------------------------------------------------------
void SETTINGS_IMPL::ErrorCallback(void * data, const char * buf)
{
    printfd(__FILE__, buf);
    SETTINGS_IMPL * settings = static_cast<SETTINGS_IMPL *>(data);
    settings->logger(buf);
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
string confFile = confDir + "/stargazer.conf";

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
        if (ParseDetailStatWritePeriod(node->getValue(0)) != 0)
            {
            strError = "Incorrect DetailStatWritePeriod value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "StatWritePeriod") == 0)
        {
        if (ParseUnsignedInRange(node->getValue(0), 1, 1440, &statWritePeriod) != 0)
            {
            strError = "Incorrect StatWritePeriod value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "ExecMsgKey") == 0)
        {
        if (ParseInt(node->getValue(0), &stgExecMsgKey) != 0)
            {
            strError = "Incorrect ExecMsgKey value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "ExecutersNum") == 0)
        {
        if (ParseUnsignedInRange(node->getValue(0), 1, 1024, &executersNum) != 0)
            {
            strError = "Incorrect ExecutersNum value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "DayFee") == 0)
        {
        if (ParseUnsignedInRange(node->getValue(0), 0, 31, &dayFee) != 0)
            {
            strError = "Incorrect DayFee value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "FullFee") == 0)
        {
        if (ParseYesNo(node->getValue(0), &fullFee) != 0)
            {
            strError = "Incorrect FullFee value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "DayResetTraff") == 0)
        {
        if (ParseUnsignedInRange(node->getValue(0), 0, 31, &dayResetTraff) != 0)
            {
            strError = "Incorrect DayResetTraff value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "SpreadFee") == 0)
        {
        if (ParseYesNo(node->getValue(0), &spreadFee) != 0)
            {
            strError = "Incorrect SpreadFee value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "FreeMbAllowInet") == 0)
        {
        if (ParseYesNo(node->getValue(0), &freeMbAllowInet) != 0)
            {
            strError = "Incorrect FreeMbAllowInet value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "DayFeeIsLastDay") == 0)
        {
        if (ParseYesNo(node->getValue(0), &dayFeeIsLastDay) != 0)
            {
            strError = "Incorrect DayFeeIsLastDay value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "WriteFreeMbTraffCost") == 0)
        {
        if (ParseYesNo(node->getValue(0), &writeFreeMbTraffCost) != 0)
            {
            strError = "Incorrect WriteFreeMbTraffCost value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        }

    if (strcasecmp(node->getName(), "ShowFeeInCash") == 0)
        {
        if (ParseYesNo(node->getValue(0), &showFeeInCash) != 0)
            {
            strError = "Incorrect ShowFeeInCash value: \'" + string(node->getValue(0)) + "\'";
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
            strError = "Incorrect MessageTimeout value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
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
            strError = "Unexpected \'" + string(node->getValue(1)) + "\'.";
            return -1;
            }

        if (storeModulesCount)
            {
            strError = "Should be only one StoreModule.";
            return -1;
            }
        storeModulesCount++;

        storeModuleSettings.moduleName = node->getValue(0);
        ParseModuleSettings(node, &storeModuleSettings.moduleParams);
        }

    if (strcasecmp(node->getName(), "Modules") == 0)
        {
        if (node->getValue(0))
            {
            strError = "Unexpected \'" + string(node->getValue(0)) + "\'.";
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
            MODULE_SETTINGS modSettings;
            modSettings.moduleParams.clear();
            modSettings.moduleName = child->getValue();

            ParseModuleSettings(child, &modSettings.moduleParams);

            modulesSettings.push_back(modSettings);

            child = child->getNextNode();
            }
        }

    node = node->getNextNode();
    }

return 0;
}
//-----------------------------------------------------------------------------
int SETTINGS_IMPL::ParseDetailStatWritePeriod(const string & detailStatPeriodStr)
{
if (detailStatPeriodStr == "1")
    {
    detailStatWritePeriod = dsPeriod_1;
    return 0;
    }
else if (detailStatPeriodStr == "1/2")
    {
    detailStatWritePeriod = dsPeriod_1_2;
    return 0;
    }
else if (detailStatPeriodStr == "1/4")
    {
    detailStatWritePeriod = dsPeriod_1_4;
    return 0;
    }
else if (detailStatPeriodStr == "1/6")
    {
    detailStatWritePeriod = dsPeriod_1_6;
    return 0;
    }

return -1;
}
//-----------------------------------------------------------------------------
