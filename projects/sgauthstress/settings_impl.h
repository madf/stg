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

#ifndef SETTINGS_IMPL_H
#define SETTINGS_IMPL_H

#include <string>
#include <vector>

#include "stg/os_int.h"

struct MODULE_SETTINGS;
class DOTCONFDocumentNode;

class SETTINGS_IMPL {
public:
                        SETTINGS_IMPL();
                        ~SETTINGS_IMPL() {}
    int                 Reload() { return 0; }
    void                SetConfFile(const std::string cf) { confFile = cf; }
    int                 ReadSettings();

    const std::string & GetStrError() const { return strError; }

    const std::string & GetServerName() const { return serverName; }
    uint16_t            GetServerPort() const { return port; }
    uint16_t            GetLocalPort() const { return localPort; }

    const std::string & GetLogin() const { return login; }
    const std::string & GetPassword() const { return password; }

    const std::string & GetConfDir() const;
    const std::string & GetModulesPath() const { return modulesPath; }
    const MODULE_SETTINGS & GetStoreModuleSettings() const { return storeModuleSettings; }

private:
    int ParseInt(const std::string & value, int * val);
    int ParseUnsigned(const std::string & value, unsigned * val);
    int ParseIntInRange(const std::string & value, int min, int max, int * val);
    int ParseUnsignedInRange(const std::string & value, unsigned min, unsigned max, unsigned * val);
    int ParseYesNo(const std::string & value, bool * val);
    int ParseModuleSettings(const DOTCONFDocumentNode * dirNameNode, std::vector<PARAM_VALUE> * params);

    std::string login;
    std::string password;
    std::string serverName;
    int         port;
    int         localPort;

    std::string confFile;
    std::string strError;
    std::string modulesPath;

    MODULE_SETTINGS storeModuleSettings;
};

#endif
