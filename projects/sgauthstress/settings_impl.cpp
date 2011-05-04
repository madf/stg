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

#include "stg/dotconfpp.h"
#include "stg/module_settings.h"
#include "stg/common.h"

#include "settings_impl.h"

SETTINGS_IMPL::SETTINGS_IMPL()
    : port(0),
      localPort(0),
      listenWebIP(0),
      refreshPeriod(0),
      daemon(false),
      noWeb(false),
      reconnect(false),
      showPid(false),
      confFile("/etc/sgauth.conf")
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
int SETTINGS_IMPL::ParseModuleSettings(const DOTCONFDocumentNode * node, std::vector<PARAM_VALUE> * params)
{
if (!node)
    return 0;

PARAM_VALUE pv;

pv.param = node->getName();

if (node->getValue(1))
    {
    strError = "Unexpected value \'" + std::string(node->getValue(1)) + "\'.";
    printfd(__FILE__, "SETTINGS_IMPL::ParseModuleSettings() - %s\n", strError.c_str());
    return -1;
    }

const char * value = node->getValue(0);

if (!value)
    {
    strError = "Module name expected.";
    printfd(__FILE__, "SETTINGS_IMPL::ParseModuleSettings() - %s\n", strError.c_str());
    return -1;
    }

const DOTCONFDocumentNode * childNode = node->getChildNode();
while (childNode)
    {
    pv.param = childNode->getName();
    int i = 0;
    while ((value = childNode->getValue(i)) != NULL)
        {
        pv.value.push_back(value);
        ++i;
        }
    params->push_back(pv);
    pv.value.clear();
    childNode = childNode->getNextNode();
    }

return 0;
}
//-----------------------------------------------------------------------------
int SETTINGS_IMPL::ReadSettings()
{
const char * requiredOptions[] = {
    "ModulesPath",
    "StoreModule",
    "Login",
    "Password",
    "ServerName",
    "ServerPort"
    NULL
    };
int storeModulesCount = 0;

DOTCONFDocument conf(DOTCONFDocument::CASEINSENSITIVE);
conf.setRequiredOptionNames(requiredOptions);

if(conf.setContent(confFile.c_str()) != 0)
    {
    strError = "Cannot read file " + confFile + ".";
    printfd(__FILE__, "SETTINGS_IMPL::ReadSettings() - %s\n", strError.c_str());
    return -1;
    }

const DOTCONFDocumentNode * node = conf.getFirstNode();

while (node)
    {
    if (strcasecmp(node->getName(), "ModulesPath") == 0)
        {
        modulesPath = node->getValue(0);
        }

    if (strcasecmp(node->getName(), "StoreModule") == 0)
        {
        if (node->getValue(1))
            {
            strError = "Unexpected \'" + std::string(node->getValue(1)) + "\'.";
            printfd(__FILE__, "SETTINGS_IMPL::ReadSettings() - %s\n", strError.c_str());
            return -1;
            }

        if (storeModulesCount)
            {
            strError = "Should be only one source StoreModule.";
            printfd(__FILE__, "SETTINGS_IMPL::ReadSettings() - %s\n", strError.c_str());
            return -1;
            }
        ++storeModulesCount;

        storeModuleSettings.moduleName = node->getValue(0);
        ParseModuleSettings(node, &storeModuleSettings.moduleParams);
        }

    node = node->getNextNode();
    }

CONFIGFILE cf(confFile);

if (cf.Error())
    {
    strError = "Cannot read file '" + confFile + "'";
    return -1;
    }

cf.ReadString("Login", &login, "/?--?--?*");
if (login == "/?--?--?*")
    {
    strError = "Parameter 'Login' not found.";
    return -1;
    }

cf.ReadString("Password", &password, "/?--?--?*");
if (login == "/?--?--?*")
    {
    strError = "Parameter 'Password' not found.";
    return -1;
    }

cf.ReadString("ServerName", &serverName, "?*?*?");
if (serverName == "?*?*?")
    {
    strError = "Parameter 'ServerName' not found.";
    return -1;
    }

cf.ReadString("ServerPort", &temp, "5555");
if (ParseIntInRange(temp, 1, 65535, &port))
    {
    strError = "Parameter 'ServerPort' is not valid.";
    return -1;
    }

cf.ReadString("LocalPort", &temp, "0");
if (ParseIntInRange(temp, 0, 65535, &localPort))
    {
    strError = "Parameter 'LocalPort' is not valid.";
    return -1;
    }

return 0;
}
