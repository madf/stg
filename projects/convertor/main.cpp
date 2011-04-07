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

 /*
 $Revision: 1.11 $
 $Date: 2010/03/25 12:32:30 $
 $Author: faust $
 */

#include <dlfcn.h>

#include <string>
#include <vector>
#include <iostream>
#include <ctime>
#include <algorithm>

#include "common.h"
#include "store.h"
#include "settings_impl.h"
#include "conffiles.h"

#include "user_stat.h"
#include "user_conf.h"
#include "corp_conf.h"
#include "service_conf.h"
#include "admin_conf.h"
#include "tariff_conf.h"
#include "settings.h"
#include "stg_message.h"

using namespace std;

volatile time_t stgTime = time(NULL);

int main(int argc, char **argv)
{
printfd(__FILE__, "Start\n");

STORE * fromStore = NULL;
STORE * toStore = NULL;

SETTINGS_IMPL * settings = NULL;

string modulePath;

MODULE_SETTINGS fromStoreSettings;
MODULE_SETTINGS toStoreSettings;

ADMIN_CONF ac;
USER_CONF uc;
USER_STAT us;
STG_MSG msg;
TARIFF_DATA td;
CORP_CONF cc;
SERVICE_CONF sc;
vector<STG_MSG_HDR> hdrs;

if (argc == 2)
    settings = new SETTINGS_IMPL(argv[1]);
else
    settings = new SETTINGS_IMPL();

if (settings->ReadSettings())
    {
    printfd(__FILE__, "Error reading settings\n");
    delete settings;
    return -1;
    }

fromStoreSettings = settings->GetSourceStoreModuleSettings();
toStoreSettings = settings->GetDestStoreModuleSettings();
modulePath = settings->GetModulesPath();

string sourcePlugin(modulePath + "/mod_" + fromStoreSettings.moduleName + ".so");
string destPlugin(modulePath + "/mod_" + toStoreSettings.moduleName + ".so");

void * src_lh = dlopen(sourcePlugin.c_str(), RTLD_NOW);
if (!src_lh)
    {
    printfd(__FILE__, "Source storage plugin loading failed: %s\n", dlerror());
    delete settings;
    return -1;
    }

void * dst_lh = dlopen(destPlugin.c_str(), RTLD_NOW);
if (!dst_lh)
    {
    printfd(__FILE__, "Destination storage plugin loading failed: %s\n", dlerror());
    delete settings;
    return -1;
    }

STORE * (*GetSourceStore)();
STORE * (*GetDestStore)();
GetSourceStore = (STORE * (*)())dlsym(src_lh, "GetStore");
if (!GetSourceStore)
    {
    printfd(__FILE__, "Source storage plugin loading failed. GetStore not found: %s\n", dlerror());
    delete settings;
    return -1;
    }
GetDestStore = (STORE * (*)())dlsym(dst_lh, "GetStore");
if (!GetDestStore)
    {
    printfd(__FILE__, "Storage plugin (firebird) loading failed. GetStore not found: %s\n", dlerror());
    delete settings;
    return -1;
    }

fromStore = GetSourceStore();
toStore = GetDestStore();

vector<string> entities;
vector<string> ready;
fromStore->SetSettings(fromStoreSettings);
fromStore->ParseSettings();
toStore->SetSettings(toStoreSettings);
toStore->ParseSettings();

printfd(__FILE__, "Importing admins:\n");
entities.erase(entities.begin(), entities.end());
ready.erase(ready.begin(), ready.end());
if (fromStore->GetAdminsList(&entities))
    {
    printfd(__FILE__, "Error getting admins list: %s\n", fromStore->GetStrError().c_str());
    dlclose(src_lh);
    dlclose(dst_lh);
    delete settings;
    return -1;
    }
if (toStore->GetAdminsList(&ready))
    {
    printfd(__FILE__, "Error getting admins list: %s\n", toStore->GetStrError().c_str());
    dlclose(src_lh);
    dlclose(dst_lh);
    delete settings;
    return -1;
    }

vector<string>::const_iterator it;
for (it = entities.begin(); it != entities.end(); ++it)
    {
    printfd(__FILE__, "\t - %s\n", it->c_str());
    if (find(ready.begin(), ready.end(), *it) == ready.end())
        if (toStore->AddAdmin(*it))
            {
            printfd(__FILE__, "Error adding admin: %s\n", toStore->GetStrError().c_str());
            dlclose(src_lh);
            dlclose(dst_lh);
            delete settings;
            return -1;
            }
    if (fromStore->RestoreAdmin(&ac, *it))
        {
        printfd(__FILE__, "Error getting admin's confi: %s\n", fromStore->GetStrError().c_str());
        dlclose(src_lh);
        dlclose(dst_lh);
        delete settings;
        return -1;
        }
    ac.login = *it;
    if (toStore->SaveAdmin(ac))
        {
        printfd(__FILE__, "Error saving admin's conf: %s\n", toStore->GetStrError().c_str());
        dlclose(src_lh);
        dlclose(dst_lh);
        delete settings;
        return -1;
        }
    }

printfd(__FILE__, "Importing tariffs:\n");
entities.erase(entities.begin(), entities.end());
ready.erase(ready.begin(), ready.end());
if (fromStore->GetTariffsList(&entities))
    {
    printfd(__FILE__, "Error getting tariffs list: %s\n", fromStore->GetStrError().c_str());
    dlclose(src_lh);
    dlclose(dst_lh);
    delete settings;
    return -1;
    }
if (toStore->GetTariffsList(&ready))
    {
    printfd(__FILE__, "Error getting tariffs list: %s\n", toStore->GetStrError().c_str());
    dlclose(src_lh);
    dlclose(dst_lh);
    delete settings;
    return -1;
    }

for (it = entities.begin(); it != entities.end(); ++it)
    {
    printfd(__FILE__, "\t - %s\n", it->c_str());
    if (find(ready.begin(), ready.end(), *it) == ready.end())
        if (toStore->AddTariff(*it))
            {
            printfd(__FILE__, "Error adding tariff: %s\n", toStore->GetStrError().c_str());
            dlclose(src_lh);
            dlclose(dst_lh);
            delete settings;
            return -1;
            }
    if (fromStore->RestoreTariff(&td, *it))
        {
        printfd(__FILE__, "Error getting tariff's data: %s\n", fromStore->GetStrError().c_str());
        dlclose(src_lh);
        dlclose(dst_lh);
        delete settings;
        return -1;
        }
    if (toStore->SaveTariff(td, *it))
        {
        printfd(__FILE__, "Error saving tariff's data: %s\n", toStore->GetStrError().c_str());
        dlclose(src_lh);
        dlclose(dst_lh);
        delete settings;
        return -1;
        }
    }

printfd(__FILE__, "Importing services:\n");
entities.erase(entities.begin(), entities.end());
ready.erase(ready.begin(), ready.end());
if (fromStore->GetServicesList(&entities))
    {
    printfd(__FILE__, "Error getting service list: %s\n", fromStore->GetStrError().c_str());
    dlclose(src_lh);
    dlclose(dst_lh);
    delete settings;
    return -1;
    }
if (toStore->GetServicesList(&ready))
    {
    printfd(__FILE__, "Error getting service list: %s\n", toStore->GetStrError().c_str());
    dlclose(src_lh);
    dlclose(dst_lh);
    delete settings;
    return -1;
    }

for (it = entities.begin(); it != entities.end(); ++it)
    {
    printfd(__FILE__, "\t - %s\n", it->c_str());
    if (find(ready.begin(), ready.end(), *it) == ready.end())
        if (toStore->AddService(*it))
            {
            printfd(__FILE__, "Error adding service: %s\n", toStore->GetStrError().c_str());
            dlclose(src_lh);
            dlclose(dst_lh);
            delete settings;
            return -1;
            }
    if (fromStore->RestoreService(&sc, *it))
        {
        printfd(__FILE__, "Error getting service's data: %s\n", fromStore->GetStrError().c_str());
        dlclose(src_lh);
        dlclose(dst_lh);
        delete settings;
        return -1;
        }
    if (toStore->SaveService(sc))
        {
        printfd(__FILE__, "Error saving service's data: %s\n", toStore->GetStrError().c_str());
        dlclose(src_lh);
        dlclose(dst_lh);
        delete settings;
        return -1;
        }
    }

printfd(__FILE__, "Importing corporations:\n");
entities.erase(entities.begin(), entities.end());
ready.erase(ready.begin(), ready.end());
if (fromStore->GetCorpsList(&entities))
    {
    printfd(__FILE__, "Error getting corporations list: %s\n", fromStore->GetStrError().c_str());
    dlclose(src_lh);
    dlclose(dst_lh);
    delete settings;
    return -1;
    }
if (toStore->GetCorpsList(&ready))
    {
    printfd(__FILE__, "Error getting corporations list: %s\n", toStore->GetStrError().c_str());
    dlclose(src_lh);
    dlclose(dst_lh);
    delete settings;
    return -1;
    }

for (it = entities.begin(); it != entities.end(); ++it)
    {
    printfd(__FILE__, "\t - %s\n", it->c_str());
    if (find(ready.begin(), ready.end(), *it) == ready.end())
        if (toStore->AddCorp(*it))
            {
            printfd(__FILE__, "Error adding corporation: %s\n", toStore->GetStrError().c_str());
            dlclose(src_lh);
            dlclose(dst_lh);
            delete settings;
            return -1;
            }
    if (fromStore->RestoreCorp(&cc, *it))
        {
        printfd(__FILE__, "Error getting corporation's data: %s\n", fromStore->GetStrError().c_str());
        dlclose(src_lh);
        dlclose(dst_lh);
        delete settings;
        return -1;
        }
    if (toStore->SaveCorp(cc))
        {
        printfd(__FILE__, "Error saving corporation's data: %s\n", toStore->GetStrError().c_str());
        dlclose(src_lh);
        dlclose(dst_lh);
        delete settings;
        return -1;
        }
    }

printfd(__FILE__, "Importing users:\n");
entities.erase(entities.begin(), entities.end());
ready.erase(ready.begin(), ready.end());
if (fromStore->GetUsersList(&entities))
    {
    printfd(__FILE__, "Error getting users list: %s\n", fromStore->GetStrError().c_str());
    dlclose(src_lh);
    dlclose(dst_lh);
    delete settings;
    return -1;
    }
if (toStore->GetUsersList(&ready))
    {
    printfd(__FILE__, "Error getting users list: %s\n", toStore->GetStrError().c_str());
    dlclose(src_lh);
    dlclose(dst_lh);
    delete settings;
    return -1;
    }

sort(ready.begin(), ready.end());
for (it = entities.begin(); it != entities.end(); ++it)
    {
    printfd(__FILE__, "\t - %s\n", it->c_str());
    if (!binary_search(ready.begin(), ready.end(), *it)) {
        if (toStore->AddUser(*it))
            {
            printfd(__FILE__, "Error adding user: %s\n", toStore->GetStrError().c_str());
            dlclose(src_lh);
            dlclose(dst_lh);
            delete settings;
            return -1;
            }
    } else {
        printfd(__FILE__, "\t\t(adding passed)\n");
    }
    if (fromStore->RestoreUserConf(&uc, *it))
        {
        printfd(__FILE__, "Error getting user's conf: %s\n", fromStore->GetStrError().c_str());
        dlclose(src_lh);
        dlclose(dst_lh);
        delete settings;
        return -1;
        }
    if (fromStore->RestoreUserStat(&us, *it))
        {
        printfd(__FILE__, "Error getting user's stat: %s\n", fromStore->GetStrError().c_str());
        dlclose(src_lh);
        dlclose(dst_lh);
        delete settings;
        return -1;
        }
    if (toStore->SaveUserConf(uc, *it))
        {
        printfd(__FILE__, "Error saving user's conf: %s\n", toStore->GetStrError().c_str());
        dlclose(src_lh);
        dlclose(dst_lh);
        delete settings;
        return -1;
        }
    if (toStore->SaveUserStat(us, *it))
        {
        printfd(__FILE__, "Error saving user's stat: %s\n", toStore->GetStrError().c_str());
        dlclose(src_lh);
        dlclose(dst_lh);
        delete settings;
        return -1;
        }
    hdrs.erase(hdrs.begin(), hdrs.end());
    if (fromStore->GetMessageHdrs(&hdrs, *it))
        {
        printfd(__FILE__, "Error getting user's messages: %s\n", fromStore->GetStrError().c_str());
        dlclose(src_lh);
        dlclose(dst_lh);
        delete settings;
        return -1;
        }
    vector<STG_MSG_HDR>::iterator mit;
    for (mit = hdrs.begin(); mit != hdrs.end(); ++mit)
        {
        if (fromStore->GetMessage(mit->id, &msg, *it))
            {
            printfd(__FILE__, "Error getting message for a user: %s\n", fromStore->GetStrError().c_str());
            dlclose(src_lh);
            dlclose(dst_lh);
            delete settings;
            return -1;
            }
        printfd(__FILE__, "\t\t * %s\n", msg.text.c_str());
        if (toStore->AddMessage(&msg, *it))
            {
            printfd(__FILE__, "Error adding message to a user: %s\n", toStore->GetStrError().c_str());
            dlclose(src_lh);
            dlclose(dst_lh);
            delete settings;
            return -1;
            }
        }
    }

dlclose(src_lh);
dlclose(dst_lh);
printfd(__FILE__, "Done\n");
delete settings;
return 0;
}
