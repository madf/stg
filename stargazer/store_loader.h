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
 $Revision: 1.3 $
 $Date: 2010/03/04 12:24:19 $
 $Author: faust $
 */

/*
 *  Header file for RAII store plugin loader
 */

#ifndef __STORE_LOADER_H__
#define __STORE_LOADER_H__

#include "stg/module_settings.h"
#include "stg/noncopyable.h"

#include <string>

class STORE;
class SETTINGS_IMPL;

class STORE_LOADER : private NONCOPYABLE {
public:
    explicit STORE_LOADER(const SETTINGS_IMPL & settings);
    ~STORE_LOADER();

    bool Load();
    bool Unload();

    STORE & GetStore() { return *plugin; }

    const std::string & GetStrError() const { return errorStr; }

private:
    STORE_LOADER(const STORE_LOADER & rvalue);
    STORE_LOADER & operator=(const STORE_LOADER & rvalue);

    bool isLoaded;
    void * handle;
    STORE * plugin;
    std::string errorStr;
    MODULE_SETTINGS storeSettings;
    std::string pluginFileName;
};

#endif
