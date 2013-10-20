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

#ifndef CORP_CONF_H
#define CORP_CONF_H

#include "resetable.h"

#include <string>

struct CORP_CONF
{
CORP_CONF() : name(), cash(0) {}
CORP_CONF(const std::string & n) : name(n), cash(0) {}
CORP_CONF(const std::string & n, double c) : name(n), cash(c) {}

std::string name;
double      cash;
};

struct CORP_CONF_RES
{
CORP_CONF_RES()
    : name(), cash()
{}

CORP_CONF_RES & operator=(const CORP_CONF & conf)
{
name = conf.name;
cash = conf.cash;
return *this;
}

CORP_CONF GetData() const
{
CORP_CONF cc;
cc.name = name.data();
cc.cash = cash.data();
return cc;
}

RESETABLE<std::string> name;
RESETABLE<double>      cash;
};

inline
bool operator==(const CORP_CONF & a, const CORP_CONF & b)
{
return a.name == b.name;
}

#endif //CORP_CONF_H
