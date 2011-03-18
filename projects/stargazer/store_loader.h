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

#include <string>

#include "module_settings.h"
#include "noncopyable.h"

class STORE;
class SETTINGS;

class STORE_LOADER : private NONCOPYABLE {
public:
    STORE_LOADER(const SETTINGS & settings);
    ~STORE_LOADER();

    bool Load();
    bool Unload();

    STORE * GetStore() { return plugin; }

    const std::string & GetStrError() const { return errorStr; }
private:
    bool isLoaded;
    void * handle;
    STORE * plugin;
    std::string errorStr;
    MODULE_SETTINGS storeSettings;
    std::string pluginFileName;
};

#endif
