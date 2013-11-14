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

#ifndef USERS_H
#define USERS_H

#include <string>

#include "notifer.h"
#include "user.h"

class ADMIN;

class USERS {
public:
    virtual ~USERS() {}
    virtual int  FindByName(const std::string & login, USER_PTR * user) = 0;

    virtual bool TariffInUse(const std::string & tariffName) const = 0;

    virtual void AddNotifierUserAdd(NOTIFIER_BASE<USER_PTR> * notifier) = 0;
    virtual void DelNotifierUserAdd(NOTIFIER_BASE<USER_PTR> * notifier) = 0;

    virtual void AddNotifierUserDel(NOTIFIER_BASE<USER_PTR> * notifier) = 0;
    virtual void DelNotifierUserDel(NOTIFIER_BASE<USER_PTR> * notifier) = 0;

    virtual int  Add(const std::string & login, const ADMIN * admin) = 0;
    virtual void Del(const std::string & login, const ADMIN * admin) = 0;

    virtual bool Authorize(const std::string & login, uint32_t ip,
                           uint32_t enabledDirs, const AUTH * auth) = 0;
    virtual bool Unauthorize(const std::string & login,
                             const AUTH * auth,
                             const std::string & reason = std::string()) = 0;

    virtual int  ReadUsers() = 0;
    virtual size_t Count() const = 0;

    virtual int  FindByIPIdx(uint32_t ip, USER_PTR * user) const = 0;
    virtual bool IsIPInIndex(uint32_t ip) const = 0;
    virtual bool IsIPInUse(uint32_t ip, const std::string & login, CONST_USER_PTR * user) const = 0;

    virtual int  OpenSearch() = 0;
    virtual int  SearchNext(int handle, USER_PTR * u) = 0;
    virtual int  CloseSearch(int handle) = 0;

    virtual int  Start() = 0;
    virtual int  Stop() = 0;
};

#endif
