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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string>

using namespace std;

#include "settings.h"
#include "common.h"

//-----------------------------------------------------------------------------
SETTINGS::SETTINGS()
    : confDir("/etc/stargazer"),
      scriptDir("/etc/stargazer"),
      pidFile("/var/run/stargazer.pid"),
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
      logger(GetStgLogger())
{
}
//-----------------------------------------------------------------------------
SETTINGS::SETTINGS(const std::string & cd)
    : confDir(cd),
      scriptDir(cd),
      pidFile(),
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
      logger(GetStgLogger())
{
}
//-----------------------------------------------------------------------------
SETTINGS::SETTINGS(const SETTINGS & rval)
    : confDir(rval.confDir),
      scriptDir(rval.scriptDir),
      detailStatWritePeriod(dsPeriod_1_6),
      statWritePeriod(10),
      dayFee(0),
      dayResetTraff(0),
      freeMbAllowInet(false),
      dayFeeIsLastDay(false),
      writeFreeMbTraffCost(false),
      logger(GetStgLogger())
{
spreadFee = rval.spreadFee;
pidFile = rval.pidFile;
stgExecMsgKey = rval.stgExecMsgKey;
executersNum = rval.executersNum;
showFeeInCash = rval.showFeeInCash;
fullFee = rval.fullFee;
monitoring = rval.monitoring;
}
//-----------------------------------------------------------------------------
SETTINGS::~SETTINGS()
{
}
//-----------------------------------------------------------------------------
int SETTINGS::ParseYesNo(const string & value, bool * val)
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
int SETTINGS::ParseInt(const string & value, int * val)
{
/*char *res;
*val = strtol(value.c_str(), &res, 10);*/
if (str2x<int>(value, *val))
    {
    strError = "Cannot convert \'" + value + "\' to integer.";
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int SETTINGS::ParseIntInRange(const string & value, int min, int max, int * val)
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
int SETTINGS::ParseModuleSettings(const DOTCONFDocumentNode * node, vector<PARAM_VALUE> * params)
{
/*if (!node)
    return 0;*/
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
    while ((value = childNode->getValue(i)) != NULL)
        {
        //printfd(__FILE__, "--> param=\'%s\' value=\'%s\'\n", childNode->getName(), value);
        pv.value.push_back(value);
        i++;
        }
    params->push_back(pv);
    pv.value.clear();
    childNode = childNode->getNextNode();
    }

/*for (unsigned i = 0; i < params->size(); i++)
    {
    printfd(__FILE__, "param \'%s\'\n", (*params)[i].param.c_str());
    for (unsigned j = 0; j < (*params)[i].value.size(); j++)
        {
        printfd(__FILE__, "value \'%s\'\n", (*params)[i].value[j].c_str());
        }
    }*/

return 0;
}
//-----------------------------------------------------------------------------
string SETTINGS::GetStrError() const
{
return strError;
}
//-----------------------------------------------------------------------------
void SETTINGS::ErrorCallback(void * data, const char * buf)
{
    printfd(__FILE__, buf);
    SETTINGS * settings = static_cast<SETTINGS *>(data);
    settings->logger(buf);
}
//-----------------------------------------------------------------------------
int SETTINGS::ReadSettings()
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
conf.setErrorCallback(SETTINGS::ErrorCallback, this);
conf.setRequiredOptionNames(requiredOptions);
string confFile = confDir + "/stargazer.conf";

//printfd(__FILE__, "Conffile: %s\n", confFile.c_str());

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
        scriptDir = node->getValue(0);
        //printfd(__FILE__, "LogFile: %s\n", logFile.c_str());
        }

    if (strcasecmp(node->getName(), "LogFile") == 0)
        {
        logFile = node->getValue(0);
        //printfd(__FILE__, "LogFile: %s\n", logFile.c_str());
        }

    if (strcasecmp(node->getName(), "PIDFile") == 0)
        {
        pidFile = node->getValue(0);
        //printfd(__FILE__, "PIDFile: %s\n", pidFile.c_str());
        }

    if (strcasecmp(node->getName(), "ModulesPath") == 0)
        {
        modulesPath = node->getValue(0);
        //printfd(__FILE__, "ModulesPath: %s\n", logFile.c_str());
        }

    if (strcasecmp(node->getName(), "Rules") == 0)
        {
        rules = node->getValue(0);
        //printfd(__FILE__, "Rules: %s\n", rules.c_str());
        }

    if (strcasecmp(node->getName(), "DetailStatWritePeriod") == 0)
        {
        if (ParseDetailStatWritePeriod(node->getValue(0)) != 0)
            {
            strError = "Incorrect DetailStatWritePeriod value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        //printfd(__FILE__, "DetailStatWritePeriod: %d\n", detailStatWritePeriod);
        }

    if (strcasecmp(node->getName(), "StatWritePeriod") == 0)
        {
        if (ParseIntInRange(node->getValue(0), 1, 1440, &statWritePeriod) != 0)
            {
            strError = "Incorrect StatWritePeriod value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        //printfd(__FILE__, "StatWritePeriod: %d\n", statWritePeriod);
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
        if (ParseIntInRange(node->getValue(0), 1, 1024, &executersNum) != 0)
            {
            strError = "Incorrect ExecutersNum value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        //printfd(__FILE__, "DayResetTraff: %d\n", dayResetTraff);
        }

    /*if (strcasecmp(node->getName(), "ExecutersWaitTimeout") == 0)
        {
        if (ParseIntInRange(node->getValue(0), 1, 600, &executersWaitTimeout) != 0)
            {
            strError = "Incorrect ExecutersWaitTimeout value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        //printfd(__FILE__, "DayResetTraff: %d\n", dayResetTraff);
        }*/

    if (strcasecmp(node->getName(), "DayFee") == 0)
        {
        if (ParseIntInRange(node->getValue(0), 0, 31, &dayFee) != 0)
            {
            strError = "Incorrect DayFee value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        //printfd(__FILE__, "DayFee: %d\n", dayFee);
        }

    if (strcasecmp(node->getName(), "FullFee") == 0)
        {
        if (ParseYesNo(node->getValue(0), &fullFee) != 0)
            {
            strError = "Incorrect FullFee value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        //printfd(__FILE__, "DayFee: %d\n", dayFee);
        }

    if (strcasecmp(node->getName(), "DayResetTraff") == 0)
        {
        if (ParseIntInRange(node->getValue(0), 0, 31, &dayResetTraff) != 0)
            {
            strError = "Incorrect DayResetTraff value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        //printfd(__FILE__, "DayResetTraff: %d\n", dayResetTraff);
        }

    if (strcasecmp(node->getName(), "SpreadFee") == 0)
        {
        if (ParseYesNo(node->getValue(0), &spreadFee) != 0)
            {
            strError = "Incorrect SpreadFee value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        //printfd(__FILE__, "SpreadFee: %d\n", spreadFee);
        }

    if (strcasecmp(node->getName(), "FreeMbAllowInet") == 0)
        {
        if (ParseYesNo(node->getValue(0), &freeMbAllowInet) != 0)
            {
            strError = "Incorrect FreeMbAllowInet value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        //printfd(__FILE__, "FreeMbAllowInet: %d\n", freeMbAllowInet);
        }

    if (strcasecmp(node->getName(), "DayFeeIsLastDay") == 0)
        {
        if (ParseYesNo(node->getValue(0), &dayFeeIsLastDay) != 0)
            {
            strError = "Incorrect DayFeeIsLastDay value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        //printfd(__FILE__, "DayFeeIsLastDay: %d\n", dayFeeIsLastDay);
        }

    if (strcasecmp(node->getName(), "WriteFreeMbTraffCost") == 0)
        {
        if (ParseYesNo(node->getValue(0), &writeFreeMbTraffCost) != 0)
            {
            strError = "Incorrect WriteFreeMbTraffCost value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        //printfd(__FILE__, "WriteFreeMbTraffCost: %d\n", writeFreeMbTraffCost);
        }

    if (strcasecmp(node->getName(), "ShowFeeInCash") == 0)
        {
        if (ParseYesNo(node->getValue(0), &showFeeInCash) != 0)
            {
            strError = "Incorrect ShowFeeInCash value: \'" + string(node->getValue(0)) + "\'";
            return -1;
            }
        //printfd(__FILE__, "ShowFeeInCash: %d\n", showFeeInCash);
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

    if (strcasecmp(node->getName(), "DirNames") == 0)
        {
        // Мы внутри секции DirNames
        const DOTCONFDocumentNode * child = node->getChildNode();
        if (child)
            {
            const DOTCONFDocumentNode * dirNameNode;
            for (int i = 0; i < DIR_NUM; i++)
                {
                char strDirName[12];
                sprintf(strDirName, "DirName%d", i);
                dirNameNode = conf.findNode(strDirName, node);
                if (dirNameNode && dirNameNode->getValue(0))
                    {
                    dirName[i] = dirNameNode->getValue(0);
                    //printfd(__FILE__, "dirName[%d]: %s\n", i, dirName[i].c_str());
                    }
                }
            }
        }

    if (strcasecmp(node->getName(), "StoreModule") == 0)
        {
        // Мы внутри секции StoreModule
        //printfd(__FILE__, "StoreModule\n");

        if (node->getValue(1))
            {
            // StoreModule должен иметь 1 атрибут
            strError = "Unexpected \'" + string(node->getValue(1)) + "\'.";
            return -1;
            }

        if (storeModulesCount)
            {
            // Должен быть только один модуль StoreModule!
            strError = "Should be only one StoreModule.";
            return -1;
            }
        storeModulesCount++;

        //storeModuleSettings.clear(); //TODO To make constructor
        //printfd(__FILE__, "StoreModule %s\n", node->getValue());
        storeModuleSettings.moduleName = node->getValue(0);
        ParseModuleSettings(node, &storeModuleSettings.moduleParams);
        }

    // Читаем настройки всех оставшихся модулей.
    if (strcasecmp(node->getName(), "Modules") == 0)
        {
        // Мы внутри секции Modules
        if (node->getValue(0))
            {
            // Modules не должен иметь атрибуов
            strError = "Unexpected \'" + string(node->getValue(0)) + "\'.";
            return -1;
            }
        const DOTCONFDocumentNode * child = node->getChildNode();
        while (child)
            {
            // Мы внутри секции
            //printfd(__FILE__, "Module \'%s\'\n", child->getValue(0));
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

//sort(modulesSettings.begin(), modulesSettings.end());
//modulesSettings.erase(unique(modulesSettings.begin(), modulesSettings.end()), modulesSettings.end());

return 0;
}
//-----------------------------------------------------------------------------
int SETTINGS::ParseDetailStatWritePeriod(const string & detailStatPeriodStr)
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
int SETTINGS::Reload ()
{
return ReadSettings();
}
//-----------------------------------------------------------------------------
const MODULE_SETTINGS & SETTINGS::GetStoreModuleSettings() const
{
return storeModuleSettings;
}
//-----------------------------------------------------------------------------
const vector<MODULE_SETTINGS> & SETTINGS::GetModulesSettings() const
{
return modulesSettings;
}
//-----------------------------------------------------------------------------
/*int SETTINGS::GetExecutersWaitTimeout() const
{
return executersWaitTimeout;
}*/
//-----------------------------------------------------------------------------
