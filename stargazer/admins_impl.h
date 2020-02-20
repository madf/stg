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

#include "admin_impl.h"

#include "stg/admins.h"
#include "stg/admin.h"
#include "stg/locker.h"
#include "stg/store.h"
#include "stg/noncopyable.h"
#include "stg/logger.h"

#include <vector>
#include <map>
#include <string>

#include <pthread.h>

namespace STG
{

class AdminsImpl : public Admins {
    public:
        explicit AdminsImpl(Store * st);
        virtual ~AdminsImpl() {}

        int           Add(const std::string & login, const Admin * admin);
        int           Del(const std::string & login, const Admin * admin);
        int           Change(const AdminConf & ac, const Admin * admin);
        const Admin * GetSysAdmin() const { return &stg; }
        const Admin * GetNoAdmin() const { return &noAdmin; }
        bool          Find(const std::string & l, Admin ** admin);
        bool          Exists(const std::string & login) const;
        bool          Correct(const std::string & login,
                              const std::string & password,
                              Admin ** admin);
        const std::string & GetStrError() const { return strError; }

        size_t        Count() const { return data.size(); }

        int OpenSearch() const;
        int SearchNext(int, AdminConf * ac) const;
        int CloseSearch(int) const;

    private:
        AdminsImpl(const AdminsImpl & rvalue);
        AdminsImpl & operator=(const AdminsImpl & rvalue);

        typedef std::vector<AdminImpl>::iterator admin_iter;
        typedef std::vector<AdminImpl>::const_iterator const_admin_iter;

        int             Read();

        AdminImpl              stg;
        AdminImpl              noAdmin;
        std::vector<AdminImpl> data;
        Store *                 store;
        Logger &            WriteServLog;
        mutable std::map<int, const_admin_iter> searchDescriptors;
        mutable unsigned int    handle;
        mutable pthread_mutex_t mutex;
        std::string             strError;
};

}
