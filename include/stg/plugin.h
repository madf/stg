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

#pragma once

#include <string>
#include <cstdint>

namespace STG
{

struct TraffCounter;
struct Settings;
struct Store;
struct Admins;
class Users;
class Tariffs;
struct Services;
struct Corporations;
struct ModuleSettings;

struct Plugin {
    virtual ~Plugin() = default;

    virtual void               SetUsers(Users*) {}
    virtual void               SetTariffs(Tariffs*) {}
    virtual void               SetAdmins(Admins*) {}
    virtual void               SetServices(Services*) {}
    virtual void               SetCorporations(Corporations*) {}
    virtual void               SetTraffcounter(TraffCounter*) {}
    virtual void               SetStore(Store*) {}
    virtual void               SetStgSettings(const Settings*) {}
    virtual void               SetSettings(const ModuleSettings&) {}
    virtual int                ParseSettings() = 0;

    virtual int                Start() = 0;
    virtual int                Stop() = 0;
    virtual int                Reload(const ModuleSettings&) = 0;
    virtual bool               IsRunning() = 0;
    virtual const std::string& GetStrError() const = 0;
    virtual std::string        GetVersion() const = 0;
    virtual uint16_t           GetStartPosition() const = 0;
    virtual uint16_t           GetStopPosition() const = 0;
};

}
