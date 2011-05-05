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

#include <cstring>
#include <cassert>
#include <vector>

#include "stg/dotconfpp.h"
#include "stg/module_settings.h"
#include "stg/common.h"

#include "settings.h"

SETTINGS::SETTINGS()
    : port(0),
      localPort(0),
      confFile("/etc/sgauth.conf")
{
}
//-----------------------------------------------------------------------------
int ParseYesNo(const std::string & value, bool * val)
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

return -1;
}
//-----------------------------------------------------------------------------
int ParseInt(const std::string & value, int * val)
{
if (str2x<int>(value, *val))
    return -1;

return 0;
}
//-----------------------------------------------------------------------------
int ParseUnsigned(const std::string & value, unsigned * val)
{
if (str2x<unsigned>(value, *val))
    return -1;

return 0;
}
//-----------------------------------------------------------------------------
int ParseIntInRange(const std::string & value, int min, int max, int * val)
{
if (ParseInt(value, val) != 0)
    return -1;

if (*val < min || *val > max)
    return -1;

return 0;
}
//-----------------------------------------------------------------------------
int ParseUnsignedInRange(const std::string & value, unsigned min, unsigned max, unsigned * val)
{
if (ParseUnsigned(value, val) != 0)
    return -1;

if (*val < min || *val > max)
    return -1;

return 0;
}
//-----------------------------------------------------------------------------
int ParseModuleSettings(const DOTCONFDocumentNode * node, std::vector<PARAM_VALUE> * params)
{
assert(node && "DOTCONFDocumentNode must not be NULL!");

const DOTCONFDocumentNode * childNode = node->getChildNode();
while (childNode)
    {
    PARAM_VALUE pv;
    pv.param = childNode->getName();
    int i = 0;
    const char * value;
    while ((value = childNode->getValue(i++)) != NULL)
        pv.value.push_back(value);
    params->push_back(pv);
    childNode = childNode->getNextNode();
    }

return 0;
}
//-----------------------------------------------------------------------------
int SETTINGS::ReadSettings()
{
const char * requiredOptions[] = {
    "ModulesPath",
    "StoreModule",
    "Login",
    "Password",
    "ServerName",
    "ServerPort",
    NULL
    };

DOTCONFDocument conf(DOTCONFDocument::CASEINSENSITIVE);
conf.setRequiredOptionNames(requiredOptions);

if(conf.setContent(confFile.c_str()) != 0)
    {
    strError = "Cannot read file " + confFile + ".";
    printfd(__FILE__, "SETTINGS::ReadSettings() - %s\n", strError.c_str());
    return -1;
    }

const DOTCONFDocumentNode * node = conf.getFirstNode();

int storeModulesCount = 0;
while (node)
    {
    if (strcasecmp(node->getName(), "ModulesPath") == 0)
        modulesPath = node->getValue(0);
    else if (strcasecmp(node->getName(), "Login") == 0)
        login = node->getValue(0);
    else if (strcasecmp(node->getName(), "Password") == 0)
        password = node->getValue(0);
    else if (strcasecmp(node->getName(), "ServerName") == 0)
        serverName = node->getValue(0);
    else if (strcasecmp(node->getName(), "ServerPort") == 0)
        {
        if (ParseIntInRange(node->getValue(0), 1, 65535, &port))
            {
            strError = "Parameter 'ServerPort' is not valid.";
            printfd(__FILE__, "SETTINGS::ReadSettings() - %s\n", strError.c_str());
            return -1;
            }
        }
    else if (strcasecmp(node->getName(), "LocalPort") == 0)
        {
        if (ParseIntInRange(node->getValue(0), 0, 65535, &localPort))
            {
            strError = "Parameter 'LocalPort' is not valid.";
            printfd(__FILE__, "SETTINGS::ReadSettings() - %s\n", strError.c_str());
            return -1;
            }
        }
    else if (strcasecmp(node->getName(), "StoreModule") == 0)
        {
        if (node->getValue(1))
            {
            strError = "Unexpected \'" + std::string(node->getValue(1)) + "\'.";
            printfd(__FILE__, "SETTINGS::ReadSettings() - %s\n", strError.c_str());
            return -1;
            }

        if (storeModulesCount)
            {
            strError = "Should be only one source StoreModule.";
            printfd(__FILE__, "SETTINGS::ReadSettings() - %s\n", strError.c_str());
            return -1;
            }
        ++storeModulesCount;

        storeModuleSettings.moduleName = node->getValue(0);
        if (ParseModuleSettings(node, &storeModuleSettings.moduleParams))
            {
            strError = "Failed to parse store module settings";
            printfd(__FILE__, "SETTINGS::ReadSettings() - %s\n", strError.c_str());
            return -1;
            }
        }

    node = node->getNextNode();
    }

return 0;
}
