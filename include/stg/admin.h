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

#ifndef ADMIN_H
#define ADMIN_H

#include <string>

#include "admin_conf.h"
#include "os_int.h"

class ADMIN {
public:
    virtual ~ADMIN() {}
    virtual const std::string & GetPassword() const = 0;
    virtual const std::string & GetLogin() const = 0;
    virtual PRIV const *        GetPriv() const = 0;
    virtual uint32_t            GetPrivAsInt() const = 0;
    virtual const ADMIN_CONF &  GetConf() const = 0;
    virtual uint32_t            GetIP() const = 0;
    virtual std::string         GetIPStr() const = 0;
    virtual void                SetIP(uint32_t ip) = 0;
    virtual const std::string   GetLogStr() const = 0;
};

#endif
