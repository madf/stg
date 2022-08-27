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

#pragma once

#include "subscriptions.h"

#include <string>

#include <cstdint>

namespace STG
{

class Admin;
class User;
struct Auth;

class Users
{
    public:
        virtual ~Users() = default;

        using UserPtr = User*;
        using ConstUserPtr = const User*;

        virtual int  FindByName(const std::string& login, UserPtr* user) = 0;
        virtual int  FindByName(const std::string& login, ConstUserPtr* user) const = 0;
        virtual bool Exists(const std::string& login) const = 0;

        virtual bool TariffInUse(const std::string& tariffName) const = 0;

        template <typename F>
        auto onAdd(F&& f) { return m_onAddCallbacks.add(std::forward<F>(f)); }
        template <typename F>
        auto onDel(F&& f) { return m_onDelCallbacks.add(std::forward<F>(f)); }

        virtual int  Add(const std::string& login, const Admin* admin) = 0;
        virtual void Del(const std::string& login, const Admin* admin) = 0;

        virtual bool Authorize(const std::string& login, uint32_t ip,
                               uint32_t enabledDirs, const Auth* auth) = 0;
        virtual bool Unauthorize(const std::string& login,
                                 const Auth* auth,
                                 const std::string& reason = {}) = 0;

        virtual int  ReadUsers() = 0;
        virtual size_t Count() const = 0;

        virtual int  FindByIPIdx(uint32_t ip, User** user) const = 0;
        virtual bool IsIPInIndex(uint32_t ip) const = 0;
        virtual bool IsIPInUse(uint32_t ip, const std::string & login, const User** user) const = 0;

        virtual unsigned int  OpenSearch() = 0;
        virtual int  SearchNext(int handle, User** u) = 0;
        virtual int  CloseSearch(int handle) = 0;

        virtual int  Start() = 0;
        virtual int  Stop() = 0;

    protected:
        Subscriptions<UserPtr> m_onAddCallbacks;
        Subscriptions<UserPtr> m_onDelCallbacks;
};

}
