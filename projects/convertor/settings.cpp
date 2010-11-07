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
$Revision: 1.6 $
$Date: 2009/06/22 16:26:54 $
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
SETTINGS::SETTINGS(const char * cf)
{
confFile = string(cf);
}
//-----------------------------------------------------------------------------
SETTINGS::~SETTINGS()
{

}
//-----------------------------------------------------------------------------
/*
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
char *res;
*val = strtol(value.c_str(), &res, 10);
if (*res != 0)
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
*/
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
int SETTINGS::ReadSettings()
{
const char * requiredOptions[] = {
    "ModulesPath",
    "SourceStoreModule",
    "DestStoreModule",
    NULL
    };
int sourceStoreModulesCount = 0;
int destStoreModulesCount = 0;

DOTCONFDocument conf(DOTCONFDocument::CASEINSENSITIVE);
conf.setRequiredOptionNames(requiredOptions);

//printfd(__FILE__, "Conffile: %s\n", confFile.c_str());

if(conf.setContent(confFile.c_str()) != 0)
    {
    strError = "Cannot read file " + confFile + ".";
    return -1;
    }

const DOTCONFDocumentNode * node = conf.getFirstNode();

while (node)
    {
    if (strcasecmp(node->getName(), "ModulesPath") == 0)
        {
        modulesPath = node->getValue(0);
        //printfd(__FILE__, "ModulesPath: %s\n", logFile.c_str());
        }

    if (strcasecmp(node->getName(), "SourceStoreModule") == 0)
        {
        // Мы внутри секции StoreModule
        //printfd(__FILE__, "StoreModule\n");

        if (node->getValue(1))
            {
            // StoreModule должен иметь 1 атрибут
            strError = "Unexpected \'" + string(node->getValue(1)) + "\'.";
            return -1;
            }

        if (sourceStoreModulesCount)
            {
            // Должен быть только один модуль StoreModule!
            strError = "Should be only one source StoreModule.";
            return -1;
            }
        sourceStoreModulesCount++;

        //storeModuleSettings.clear(); //TODO To make constructor
        //printfd(__FILE__, "StoreModule %s\n", node->getValue());
        sourceStoreModuleSettings.moduleName = node->getValue(0);
        ParseModuleSettings(node, &sourceStoreModuleSettings.moduleParams);
        }

    if (strcasecmp(node->getName(), "DestStoreModule") == 0)
        {
        // Мы внутри секции StoreModule
        //printfd(__FILE__, "StoreModule\n");

        if (node->getValue(1))
            {
            // StoreModule должен иметь 1 атрибут
            strError = "Unexpected \'" + string(node->getValue(1)) + "\'.";
            return -1;
            }

        if (destStoreModulesCount)
            {
            // Должен быть только один модуль StoreModule!
            strError = "Should be only one dest StoreModule.";
            return -1;
            }
        destStoreModulesCount++;

        //storeModuleSettings.clear(); //TODO To make constructor
        //printfd(__FILE__, "StoreModule %s\n", node->getValue());
        destStoreModuleSettings.moduleName = node->getValue(0);
        ParseModuleSettings(node, &destStoreModuleSettings.moduleParams);
        }

    node = node->getNextNode();
    }

//sort(modulesSettings.begin(), modulesSettings.end());
//modulesSettings.erase(unique(modulesSettings.begin(), modulesSettings.end()), modulesSettings.end());

return 0;
}
//-----------------------------------------------------------------------------
int SETTINGS::Reload ()
{
return ReadSettings();
}
//-----------------------------------------------------------------------------
const MODULE_SETTINGS & SETTINGS::GetSourceStoreModuleSettings() const
{
return sourceStoreModuleSettings;
}
//-----------------------------------------------------------------------------
const MODULE_SETTINGS & SETTINGS::GetDestStoreModuleSettings() const
{
return destStoreModuleSettings;
}
//-----------------------------------------------------------------------------
const string & SETTINGS::GetModulesPath() const
{
return modulesPath;
}
//-----------------------------------------------------------------------------

