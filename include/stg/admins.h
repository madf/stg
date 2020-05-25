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

#include <string>
#include <functional>

namespace STG
{

struct AdminConf;
class Admin;

struct Admins
{
    virtual ~Admins() = default;

    virtual int add(const std::string& login, const Admin& admin) = 0;
    virtual int del(const std::string& login, const Admin& admin) = 0;
    virtual int change(const AdminConf& ac, const Admin& admin) = 0;
    virtual const Admin& sysAdmin() const = 0;
    virtual const Admin& noAdmin() const = 0;
    virtual bool find(const std::string& login, Admin** admin) = 0;
    virtual bool exists(const std::string& login) const = 0;
    virtual bool correct(const std::string& login,
                         const std::string& password,
                         Admin** admin) = 0;
    virtual const std::string& strError() const = 0;
    virtual size_t count() const = 0;
    virtual void fmap(std::function<void (const Admin&)> callback) const = 0;
};

}
