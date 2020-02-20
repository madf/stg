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

#pragma once

#include "stg/admin.h"
#include "stg/admin_conf.h"

#include <string>
#include <cstdint>

namespace STG
{

class AdminImpl : public Admin {
    public:
          AdminImpl() noexcept : ip(0) {}

          explicit AdminImpl(const AdminConf& ac) noexcept : conf(ac), ip(0) {}
          AdminImpl(const Priv& priv,
                     const std::string& login,
                     const std::string& password) noexcept
              : conf(priv, login, password), ip(0)
          {}

          AdminImpl(const AdminImpl&) = default;
          AdminImpl& operator=(const AdminImpl&) = default;
          AdminImpl(AdminImpl&&) = default;
          AdminImpl& operator=(AdminImpl&&) = default;

          AdminImpl& operator=(const AdminConf& ac) noexcept { conf = ac; return *this; }
          bool       operator==(const AdminImpl& rhs) const noexcept { return conf.login == rhs.conf.login; }
          bool       operator!=(const AdminImpl& rhs) const noexcept { return !(*this == rhs); }
          bool       operator<(const AdminImpl& rhs) const noexcept { return conf.login < rhs.conf.login; }
          //bool       operator<=(const AdminImpl & rhs) const;

          const std::string& GetPassword() const override { return conf.password; }
          const std::string& GetLogin() const override { return conf.login; }
          const Priv*        GetPriv() const override { return &conf.priv; }
          uint32_t           GetPrivAsInt() const override { return conf.priv.toInt(); }
          const AdminConf&   GetConf() const override { return conf; }
          void               Print() const;
          uint32_t           GetIP() const override { return ip; }
          std::string        GetIPStr() const override;
          void               SetIP(uint32_t v) override { ip = v; }
          const std::string  GetLogStr() const override;

    private:
          AdminConf conf;
          uint32_t  ip;
};

}
