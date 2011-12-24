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

#ifndef ADMINS_H
#define ADMINS_H

#include <string>

#include "admin.h"
#include "admin_conf.h"

class ADMINS {
public:
    virtual ~ADMINS() {}
    virtual int Add(const std::string & login, const ADMIN * admin) = 0;
    virtual int Del(const std::string & login, const ADMIN * admin) = 0;
    virtual int Change(const ADMIN_CONF & ac, const ADMIN * admin) = 0;
    virtual const ADMIN * GetSysAdmin() const = 0;
    virtual const ADMIN * GetNoAdmin() const = 0;
    virtual bool Find(const std::string & l, ADMIN ** admin) = 0;
    virtual bool Exists(const std::string & login) const = 0;
    virtual bool Correct(const std::string & login,
                         const std::string & password,
                         ADMIN ** admin) = 0;
    virtual const std::string & GetStrError() const = 0;
    virtual size_t Count() const = 0;

    virtual int OpenSearch() const = 0;
    virtual int SearchNext(int, ADMIN_CONF * ac) const = 0;
    virtual int CloseSearch(int) const = 0;
};

#endif
