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
 *    Author : Maksym Mamontov <stg@madf.info>
 */

#ifndef SERVICES_H
#define SERVICES_H

#include "service_conf.h"

#include <string>

class ADMIN;

class SERVICES {
public:
    virtual ~SERVICES() {}
    virtual int Add(const SERVICE_CONF & service, const ADMIN * admin) = 0;
    virtual int Del(const std::string & name, const ADMIN * admin) = 0;
    virtual int Change(const SERVICE_CONF & service, const ADMIN * admin) = 0;
    virtual bool Find(const std::string & name, SERVICE_CONF * service) const = 0;
    virtual bool Find(const std::string & name, SERVICE_CONF_RES * service) const = 0;
    virtual bool Exists(const std::string & name) const = 0;
    virtual const std::string & GetStrError() const = 0;
    virtual size_t Count() const = 0;

    virtual int OpenSearch() const = 0;
    virtual int SearchNext(int, SERVICE_CONF * service) const = 0;
    virtual int CloseSearch(int) const = 0;
};

#endif
