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

#ifndef ADMIN_IMPL_H
#define ADMIN_IMPL_H

#include <string>

#include "stg/admin.h"
#include "stg/os_int.h"
#include "stg/admin_conf.h"
#include "stg/logger.h"

class ADMIN_IMPL : public ADMIN {
public:
      ADMIN_IMPL();
      ADMIN_IMPL(const ADMIN_CONF & ac);
      ADMIN_IMPL(const PRIV & priv,
                 const std::string & login,
                 const std::string & password);
      virtual ~ADMIN_IMPL() {}

      ADMIN_IMPL & operator=(const ADMIN_IMPL &);
      ADMIN_IMPL & operator=(const ADMIN_CONF &);
      bool         operator==(const ADMIN_IMPL & rhs) const;
      bool         operator!=(const ADMIN_IMPL & rhs) const;
      bool         operator<(const ADMIN_IMPL & rhs) const;
      bool         operator<=(const ADMIN_IMPL & rhs) const;

      const std::string & GetPassword() const { return conf.password; }
      const std::string & GetLogin() const { return conf.login; }
      PRIV const *        GetPriv() const { return &conf.priv; }
      uint32_t            GetPrivAsInt() const { return conf.priv.ToInt(); }
      const ADMIN_CONF &  GetConf() const { return conf; }
      void                Print() const;
      uint32_t            GetIP() const { return ip; }
      std::string         GetIPStr() const;
      void                SetIP(uint32_t v) { ip = v; }
      const std::string   GetLogStr() const;

private:
      ADMIN_CONF        conf;
      uint32_t          ip;
      STG_LOGGER &      WriteServLog;
};

#endif
