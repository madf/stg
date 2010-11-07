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
 *    Date: 27.10.2002
 */

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

 /*
 $Revision: 1.14 $
 $Date: 2010/10/04 20:15:43 $
 $Author: faust $
 */

#ifndef ADMIN_H
#define ADMIN_H

#include <string>

#include "os_int.h"
#include "admin_conf.h"
#include "stg_logger.h"

using namespace std;

//-----------------------------------------------------------------------------
class ADMIN
{
public:
      ADMIN();
      ADMIN(const ADMIN_CONF & ac);
      ADMIN(const PRIV & priv,
            const std::string & login,
            const std::string & password);
      ~ADMIN() {};

      ADMIN &           operator=(const ADMIN &);
      ADMIN &           operator=(const ADMIN_CONF &);
      bool              operator==(const ADMIN & rhs) const;
      bool              operator!=(const ADMIN & rhs) const;
      bool              operator<(const ADMIN & rhs) const;
      bool              operator<=(const ADMIN & rhs) const;

      const string &    GetPassword() const { return conf.password; };
      const string &    GetLogin() const { return conf.login; };
      PRIV const *      GetPriv() const { return &conf.priv; };
      uint16_t          GetPrivAsInt() const { return conf.priv.ToInt(); };
      const ADMIN_CONF & GetConf() const { return conf; };
      void              PrintAdmin() const;
      uint32_t          GetAdminIP() const { return ip; };
      string            GetAdminIPStr() const;
      void              SetAdminIP(uint32_t ip) { ADMIN::ip = ip; };
      const string      GetLogStr() const;

private:
      ADMIN_CONF        conf;
      uint32_t          ip;
      STG_LOGGER &      WriteServLog;
};
//-----------------------------------------------------------------------------

#endif
