 /*
 $Revision: 1.7 $
 $Date: 2010/10/07 18:43:21 $
 */

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
 *    Date: 07.11.2007
 */

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

/*
 $Revision: 1.7 $
 $Date: 2010/10/07 18:43:21 $
 $Author: faust $
 */

#ifndef TARIFFS_IMPL_H
#define TARIFFS_IMPL_H

#include <pthread.h>

#include <string>
#include <list>

#include "stg/tariff.h"
#include "stg/tariffs.h"
#include "stg/tariff_conf.h"
#include "tariff_impl.h"

#define TARIFF_DAY     0
#define TARIFF_NIGHT   1

class STORE;
class STG_LOGGER;
class ADMIN;

class TARIFFS_IMPL : public TARIFFS {
public:
    TARIFFS_IMPL(STORE * store);
    virtual ~TARIFFS_IMPL();
    int     ReadTariffs ();
    const TARIFF * FindByName(const std::string & name) const;
    const TARIFF * GetNoTariff() const { return &noTariff; };
    size_t  Count() const;
    int     Del(const std::string & name, const ADMIN * admin);
    int     Add(const std::string & name, const ADMIN * admin);
    int     Chg(const TARIFF_DATA & td, const ADMIN * admin);

    void    GetTariffsData(std::list<TARIFF_DATA> * tdl);

    const std::string & GetStrError() const { return strError; }
private:
    std::list<TARIFF_IMPL>  tariffs;
    STORE *                 store;
    STG_LOGGER &            WriteServLog;
    mutable pthread_mutex_t mutex;
    std::string             strError;
    TARIFF_IMPL             noTariff;
};

#endif
