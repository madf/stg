 /*
 $Revision: 1.6 $
 $Date: 2009/06/22 16:26:54 $
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
class SETTINGS
{
public:
    SETTINGS(const char * cf = "./convertor.conf");
    ~SETTINGS();
    int Reload();
    int ReadSettings();

    string GetStrError() const;

    const string &  GetConfDir() const;

    const string &  GetModulesPath() const;
    const MODULE_SETTINGS         & GetSourceStoreModuleSettings() const;
    const MODULE_SETTINGS         & GetDestStoreModuleSettings() const;

private:

    //int ParseInt(const string & value, int * val);
    //int ParseIntInRange(const string & value, int min, int max, int * val);
    //int ParseYesNo(const string & value, bool * val);

    int ParseModuleSettings(const DOTCONFDocumentNode * dirNameNode, vector<PARAM_VALUE> * params);

    string      strError;
    //////////settings
    string      modulesPath;
    string      confFile;

    MODULE_SETTINGS         sourceStoreModuleSettings;
    MODULE_SETTINGS         destStoreModuleSettings;
};
//-----------------------------------------------------------------------------
#endif

