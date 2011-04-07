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

#ifndef SETTINGS_IMPL_H
#define SETTINGS_IMPL_H

#include <string>
#include <vector>

class MODULE_SETTINGS;
class DOTCONFDocumentNode;

class SETTINGS_IMPL {
public:
    SETTINGS_IMPL() : confFile("./convertor.conf") {}
    SETTINGS_IMPL(const std::string & cf) : confFile(cf) {}
    ~SETTINGS_IMPL() {}
    int ReadSettings();

    std::string GetStrError() const { return strError; }

    const std::string & GetConfDir() const;

    const std::string & GetModulesPath() const { return modulesPath; }
    const MODULE_SETTINGS & GetSourceStoreModuleSettings() const { return sourceStoreModuleSettings; }
    const MODULE_SETTINGS & GetDestStoreModuleSettings() const { return destStoreModuleSettings; }

private:
    int ParseModuleSettings(const DOTCONFDocumentNode * dirNameNode, std::vector<PARAM_VALUE> * params);

    std::string strError;
    std::string modulesPath;
    std::string confFile;

    MODULE_SETTINGS sourceStoreModuleSettings;
    MODULE_SETTINGS destStoreModuleSettings;
};

#endif
