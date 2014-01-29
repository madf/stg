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

#ifndef SERVICE_CONF_H
#define SERVICE_CONF_H

#include "resetable.h"
#include "os_int.h"

#include <string>

struct SERVICE_CONF
{
SERVICE_CONF()
    : name(), comment(), cost(0), payDay(0)
{}
SERVICE_CONF(const std::string & n)
    : name(n), comment(), cost(0), payDay(0)
{}
SERVICE_CONF(const std::string & n, double c)
    : name(n), comment(), cost(c), payDay(0)
{}
SERVICE_CONF(const std::string & n, double c, unsigned p)
    : name(n), comment(), cost(c), payDay(static_cast<uint8_t>(p))
{}
SERVICE_CONF(const std::string & n, double c,
             unsigned p, const std::string & com)
    : name(n), comment(com), cost(c), payDay(static_cast<uint8_t>(p))
{}

std::string name;
std::string comment;
double      cost;
uint8_t     payDay;
};

struct SERVICE_CONF_RES
{
SERVICE_CONF_RES()
    : name(), comment(),
      cost(), payDay()
{}

SERVICE_CONF_RES & operator=(const SERVICE_CONF & conf)
{
name = conf.name;
comment = conf.comment;
cost = conf.cost;
payDay = conf.payDay;
return *this;
}

SERVICE_CONF GetData() const
{
SERVICE_CONF sc;
sc.name = name.data();
sc.comment = comment.data();
sc.cost = cost.data();
sc.payDay = payDay.data();
return sc;
}

RESETABLE<std::string> name;
RESETABLE<std::string> comment;
RESETABLE<double>      cost;
RESETABLE<uint8_t>     payDay;
};

inline
bool operator==(const SERVICE_CONF & a, const SERVICE_CONF & b)
{
return a.name == b.name;
}

#endif //SERVICE_CONF_H

