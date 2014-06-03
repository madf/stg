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

#ifndef __STG_STGLIBS_SRVCONF_PARSER_CHG_TARIFF_H__
#define __STG_STGLIBS_SRVCONF_PARSER_CHG_TARIFF_H__

#include "base.h"

#include "stg/servconf_types.h"

#include <string>

struct TARIFF_DATA_RES;

namespace STG
{
namespace CHG_TARIFF
{

std::string Serialize(const TARIFF_DATA_RES & data, const std::string & encoding);

} // namespace CHG_TARIFF
} // namespace STG

#endif
