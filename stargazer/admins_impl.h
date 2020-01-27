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
 $Revision: 1.10 $
 $Date: 2010/10/04 20:17:12 $
 $Author: faust $
 */

#ifndef ADMINS_IMPL_H
#define ADMINS_IMPL_H

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

class ADMINS_IMPL : private NONCOPYABLE, public ADMINS {
public:
    explicit ADMINS_IMPL(STORE * st);
    virtual ~ADMINS_IMPL() {}

    int           Add(const std::string & login, const ADMIN * admin);
    int           Del(const std::string & login, const ADMIN * admin);
    int           Change(const ADMIN_CONF & ac, const ADMIN * admin);
    const ADMIN * GetSysAdmin() const { return &stg; }
    const ADMIN * GetNoAdmin() const { return &noAdmin; }
    bool          Find(const std::string & l, ADMIN ** admin);
    bool          Exists(const std::string & login) const;
    bool          Correct(const std::string & login,
                          const std::string & password,
                          ADMIN ** admin);
    const std::string & GetStrError() const { return strError; }

    size_t        Count() const { return data.size(); }

    int OpenSearch() const;
    int SearchNext(int, ADMIN_CONF * ac) const;
    int CloseSearch(int) const;

private:
    ADMINS_IMPL(const ADMINS_IMPL & rvalue);
    ADMINS_IMPL & operator=(const ADMINS_IMPL & rvalue);

    typedef std::vector<ADMIN_IMPL>::iterator admin_iter;
    typedef std::vector<ADMIN_IMPL>::const_iterator const_admin_iter;

    int             Read();

    ADMIN_IMPL              stg;
    ADMIN_IMPL              noAdmin;
    std::vector<ADMIN_IMPL> data;
    STORE *                 store;
    STG_LOGGER &            WriteServLog;
    mutable std::map<int, const_admin_iter> searchDescriptors;
    mutable unsigned int    handle;
    mutable pthread_mutex_t mutex;
    std::string             strError;
};

#endif
