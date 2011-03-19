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

#include <pthread.h>

#include <list>
#include <map>

#include "admins.h"
#include "admin.h"
#include "admin_impl.h"
#include "stg_locker.h"
#include "store.h"
#include "noncopyable.h"

class ADMINS_IMPL : private NONCOPYABLE, public ADMINS {
public:
    ADMINS_IMPL(STORE * st);
    virtual ~ADMINS_IMPL() {}

    int           Add(const string & login, const ADMIN * admin);
    int           Del(const string & login, const ADMIN * admin);
    int           Change(const ADMIN_CONF & ac, const ADMIN * admin);
    void          PrintAdmins() const;
    const ADMIN * GetSysAdmin() const { return &stg; }
    const ADMIN * GetNoAdmin() const { return &noAdmin; }
    bool          FindAdmin(const std::string & l, ADMIN ** admin);
    bool          AdminExists(const std::string & login) const;
    bool          AdminCorrect(const std::string & login,
                               const std::string & password,
                               ADMIN ** admin);
    const std::string & GetStrError() const { return strError; }

    int OpenSearch() const;
    int SearchNext(int, ADMIN_CONF * ac) const;
    int CloseSearch(int) const;

private:
    typedef list<ADMIN_IMPL>::iterator admin_iter;
    typedef list<ADMIN_IMPL>::const_iterator const_admin_iter;

    int             ReadAdmins();

    ADMIN_IMPL           stg;
    ADMIN_IMPL           noAdmin;
    list<ADMIN_IMPL>     data;
    STORE *              store;
    STG_LOGGER &         WriteServLog;
    mutable map<int, const_admin_iter> searchDescriptors;
    mutable unsigned int handle;
    mutable pthread_mutex_t mutex;
    std::string          strError;
};

#endif
