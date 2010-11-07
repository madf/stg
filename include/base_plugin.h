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


#ifndef BASE_PLUGIN_H
#define BASE_PLUGIN_H

#include <string>
#include "base_settings.h"
#include "noncopyable.h"
#include "os_int.h"

using namespace std;

class ADMINS;
class USERS;
class TARIFFS;
class TRAFFCOUNTER;
class SETTINGS;
class BASE_STORE;

//-----------------------------------------------------------------------------
class BASE_PLUGIN : private NONCOPYABLE
{
public:
    virtual                 ~BASE_PLUGIN(){};
    virtual void            SetUsers(USERS * u) = 0;
    virtual void            SetTariffs(TARIFFS * t) = 0;
    virtual void            SetAdmins(ADMINS * a) = 0;
    virtual void            SetTraffcounter(TRAFFCOUNTER * tc) = 0;
    virtual void            SetStore(BASE_STORE * st) = 0;
    virtual void            SetStgSettings(const SETTINGS * s) = 0;
    virtual void            SetSettings(const MODULE_SETTINGS & s) = 0;
    virtual int             ParseSettings() = 0;

    virtual int             Start() = 0;
    virtual int             Stop() = 0;
    virtual int             Reload() = 0;
    virtual bool            IsRunning() = 0;
    virtual const string  & GetStrError() const = 0;
    virtual const string    GetVersion() const = 0;
    virtual uint16_t        GetStartPosition() const = 0;
    virtual uint16_t        GetStopPosition() const = 0;
};
//-----------------------------------------------------------------------------
#endif


