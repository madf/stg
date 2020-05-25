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

#include "admin_conf.h"

#include "stg/common.h"

#include <string>
#include <cstdint>

namespace STG
{

class Admin
{
    public:
          Admin() noexcept : Admin(AdminConf{}) {}
          Admin(const Priv& priv,
                const std::string& login,
                const std::string& password) noexcept
              : Admin(AdminConf{priv, login, password})
          {}
          explicit Admin(const AdminConf& ac) noexcept : m_conf(ac), m_ip(0) {}

          Admin(const Admin&) = default;
          Admin& operator=(const Admin&) = default;
          Admin(Admin&&) = default;
          Admin& operator=(Admin&&) = default;

          Admin& operator=(const AdminConf& ac) noexcept { m_conf = ac; return *this; }
          bool   operator==(const Admin& rhs) const noexcept { return m_conf.login == rhs.m_conf.login; }
          bool   operator!=(const Admin& rhs) const noexcept { return !(*this == rhs); }
          bool   operator<(const Admin& rhs) const noexcept { return m_conf.login < rhs.m_conf.login; }

          const std::string& password() const { return m_conf.password; }
          const std::string& login() const { return m_conf.login; }
          const Priv&        priv() const { return m_conf.priv; }
          uint32_t           privAsInt() const { return m_conf.priv.toInt(); }
          const AdminConf&   conf() const { return m_conf; }
          uint32_t           IP() const { return m_ip; }
          std::string        IPStr() const { return inet_ntostring(m_ip); }
          void               setIP(uint32_t v) { m_ip = v; }
          const std::string  logStr() const { return "Admin \'" + m_conf.login + "\', " + IPStr() + ":"; }

    private:
          AdminConf m_conf;
          uint32_t  m_ip;
};

}
