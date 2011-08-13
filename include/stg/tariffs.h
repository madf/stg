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

#ifndef TARIFFS_H
#define TARIFFS_H

#include <string>
#include <list>

class ADMIN;
class TARIFF;
struct TARIFF_DATA;

class TARIFFS {
public:
    virtual int            ReadTariffs () = 0;
    virtual const TARIFF * FindByName(const std::string & name) const = 0;
    virtual const TARIFF * GetNoTariff() const = 0;
    virtual int            Del(const std::string & name, const ADMIN * admin) = 0;
    virtual int            Add(const std::string & name, const ADMIN * admin) = 0;
    virtual int            Chg(const TARIFF_DATA & td, const ADMIN * admin) = 0;

    virtual void           GetTariffsData(std::list<TARIFF_DATA> * tdl) = 0;

    virtual size_t         Count() const = 0;

    virtual const std::string & GetStrError() const = 0;
};

#endif
