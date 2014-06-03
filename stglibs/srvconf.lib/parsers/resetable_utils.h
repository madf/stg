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

#ifndef __STG_STGLIBS_SRVCONF_RESETABLE_UTILS_H__
#define __STG_STGLIBS_SRVCONF_RESETABLE_UTILS_H__

#include "stg/resetable.h"
#include "stg/common.h"

#include <string>
#include <ostream>

namespace STG
{

template <typename T>
inline
void appendResetable(std::ostream & stream, const std::string & name, const T & value)
{
if (!value.empty())
    stream << "<" << name << " value=\"" << value.data() << "\"/>";
}

template <typename T>
inline
void appendResetable(std::ostream & stream, const std::string & name, size_t suffix, const T & value)
{
if (!value.empty())
    stream << "<" << name << suffix << " value=\"" << value.data() << "\"/>";
}

inline
RESETABLE<std::string> MaybeEncode(const RESETABLE<std::string> & value)
{
RESETABLE<std::string> res;
if (!value.empty())
    res = Encode12str(value.data());
return res;
}

inline
RESETABLE<std::string> MaybeIconv(const RESETABLE<std::string> & value, const std::string & fromEncoding, const std::string & toEncoding)
{
RESETABLE<std::string> res;
if (!value.empty())
    res = IconvString(value.data(), fromEncoding, toEncoding);
return res;
}

} // namespace STG

#endif
