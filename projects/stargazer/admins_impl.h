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

#include "stg/admins.h"
#include "stg/admin.h"
#include "stg/store.h"
#include "stg/logger.h"

#include <vector>
#include <string>
#include <algorithm>
#include <mutex>

namespace STG
{

class AdminsImpl : public Admins
{
    public:
        explicit AdminsImpl(Store& st);

        AdminsImpl(const AdminsImpl&) = delete;
        AdminsImpl& operator=(const AdminsImpl&) = delete;

        int          add(const std::string& login, const Admin& admin) override;
        int          del(const std::string& login, const Admin& admin) override;
        int          change(const AdminConf& ac, const Admin& admin) override;
        const Admin& sysAdmin() const override { return m_stg; }
        const Admin& noAdmin() const override { return m_noAdmin; }
        bool         find(const std::string& login, Admin** admin) override;
        bool         exists(const std::string& login) const override;
        bool         correct(const std::string& login,
                             const std::string& password,
                             Admin** admin) override;

        const std::string& strError() const override { return m_strError; }

        size_t       count() const override { return m_data.size(); }

        void fmap(std::function<void (const Admin&)> callback) const override
        {
            for (const auto& admin : m_data)
                callback(admin);
        }

    private:
        void         read();
        auto         find(const std::string& login) { return std::find(m_data.begin(), m_data.end(), Admin(Priv(0), login, "")); }
        auto         find(const std::string& login) const { return std::find(m_data.begin(), m_data.end(), Admin(Priv(0), login, "")); }

        Admin              m_stg;
        Admin              m_noAdmin;
        std::vector<Admin> m_data;
        Store&             m_store;
        Logger&            WriteServLog;
        mutable std::mutex m_mutex;
        std::string        m_strError;
};

}
