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

#ifndef CORPORATIONS_IMPL_H
#define CORPORATIONS_IMPL_H

#include "stg/corporations.h"
#include "stg/corp_conf.h"
#include "stg/locker.h"
#include "stg/store.h"
#include "stg/noncopyable.h"
#include "stg/logger.h"

#include <list>
#include <map>
#include <string>

#include <pthread.h>

class ADMIN;

class CORPORATIONS_IMPL : private NONCOPYABLE, public CORPORATIONS {
public:
    explicit CORPORATIONS_IMPL(STORE * st);
    virtual ~CORPORATIONS_IMPL() {}

    int Add(const CORP_CONF & corp, const ADMIN * admin);
    int Del(const std::string & name, const ADMIN * admin);
    int Change(const CORP_CONF & corp, const ADMIN * admin);
    bool Find(const std::string & name, CORP_CONF * corp);
    bool Exists(const std::string & name) const;
    const std::string & GetStrError() const { return strError; }

    size_t Count() const { return data.size(); }

    int OpenSearch() const;
    int SearchNext(int, CORP_CONF * corp) const;
    int CloseSearch(int) const;

private:
    CORPORATIONS_IMPL(const CORPORATIONS_IMPL & rvalue);
    CORPORATIONS_IMPL & operator=(const CORPORATIONS_IMPL & rvalue);

    typedef std::list<CORP_CONF>::iterator       crp_iter;
    typedef std::list<CORP_CONF>::const_iterator const_crp_iter;

    bool Read();

    std::list<CORP_CONF> data;
    STORE * store;
    STG_LOGGER & WriteServLog;
    mutable std::map<int, const_crp_iter> searchDescriptors;
    mutable unsigned int handle;
    mutable pthread_mutex_t mutex;
    std::string strError;
};

#endif
