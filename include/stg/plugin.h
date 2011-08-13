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
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

/*
 $Revision: 1.12 $
 $Date: 2010/03/04 11:53:14 $
 $Author: faust $
*/


#ifndef PLUGIN_H
#define PLUGIN_H

#include <string>

#include "noncopyable.h"
#include "os_int.h"
#include "admins.h"
#include "users.h"
#include "tariffs.h"
#include "services.h"
#include "corporations.h"

class TRAFFCOUNTER;
class SETTINGS;
class STORE;
struct MODULE_SETTINGS;

class PLUGIN : private NONCOPYABLE {
public:
    virtual void                SetUsers(USERS *) {}
    virtual void                SetTariffs(TARIFFS *) {}
    virtual void                SetAdmins(ADMINS *) {}
    virtual void                SetServices(SERVICES *) {}
    virtual void                SetCorporations(CORPORATIONS *) {}
    virtual void                SetTraffcounter(TRAFFCOUNTER *) {}
    virtual void                SetStore(STORE *) {}
    virtual void                SetStgSettings(const SETTINGS *) {}
    virtual void                SetSettings(const MODULE_SETTINGS &) {}
    virtual int                 ParseSettings() = 0;

    virtual int                 Start() = 0;
    virtual int                 Stop() = 0;
    virtual int                 Reload() = 0;
    virtual bool                IsRunning() = 0;
    virtual const std::string & GetStrError() const = 0;
    virtual const std::string   GetVersion() const = 0;
    virtual uint16_t            GetStartPosition() const = 0;
    virtual uint16_t            GetStopPosition() const = 0;
};

#endif
