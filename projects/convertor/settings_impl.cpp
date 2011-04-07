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

#include "stg/dotconfpp.h"
#include "stg/module_settings.h"
#include "stg/common.h"

#include "settings_impl.h"

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
    "SourceStoreModule",
    "DestStoreModule",
    NULL
    };
int sourceStoreModulesCount = 0;
int destStoreModulesCount = 0;

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

    if (strcasecmp(node->getName(), "SourceStoreModule") == 0)
        {
        if (node->getValue(1))
            {
            strError = "Unexpected \'" + std::string(node->getValue(1)) + "\'.";
            printfd(__FILE__, "SETTINGS_IMPL::ReadSettings() - %s\n", strError.c_str());
            return -1;
            }

        if (sourceStoreModulesCount)
            {
            strError = "Should be only one source StoreModule.";
            printfd(__FILE__, "SETTINGS_IMPL::ReadSettings() - %s\n", strError.c_str());
            return -1;
            }
        ++sourceStoreModulesCount;

        sourceStoreModuleSettings.moduleName = node->getValue(0);
        ParseModuleSettings(node, &sourceStoreModuleSettings.moduleParams);
        }

    if (strcasecmp(node->getName(), "DestStoreModule") == 0)
        {
        if (node->getValue(1))
            {
            strError = "Unexpected \'" + std::string(node->getValue(1)) + "\'.";
            printfd(__FILE__, "SETTINGS_IMPL::ReadSettings() - %s\n", strError.c_str());
            return -1;
            }

        if (destStoreModulesCount)
            {
            strError = "Should be only one dest StoreModule.";
            printfd(__FILE__, "SETTINGS_IMPL::ReadSettings() - %s\n", strError.c_str());
            return -1;
            }
        ++destStoreModulesCount;

        destStoreModuleSettings.moduleName = node->getValue(0);
        ParseModuleSettings(node, &destStoreModuleSettings.moduleParams);
        }

    node = node->getNextNode();
    }

return 0;
}
//-----------------------------------------------------------------------------
