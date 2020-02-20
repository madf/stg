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

#pragma once

#include "stg/optional.h"
#include "stg/common.h"

#include <string>
#include <ostream>

namespace STG
{

template <typename T>
inline
void appendTag(std::ostream& stream, const std::string& name, const T& value)
{
    stream << "<" << name << " value=\"" << value << "\"/>";
}

template <typename T>
inline
void appendTag(std::ostream& stream, const std::string& name, size_t suffix, const T& value)
{
    stream << "<" << name << suffix << " value=\"" << value << "\"/>";
}

template <>
inline
void appendTag<uint8_t>(std::ostream& stream, const std::string& name, const uint8_t& value)
{
    stream << "<" << name << " value=\"" << static_cast<unsigned>(value) << "\"/>";
}

template <>
inline
void appendTag<int8_t>(std::ostream& stream, const std::string& name, const int8_t& value)
{
    stream << "<" << name << " value=\"" << static_cast<int>(value) << "\"/>";
}

template <typename T>
inline
void appendAttr(std::ostream& stream, const std::string& name, const T& value)
{
    stream << " " << name << "=\"" << value << "\"";
}

template <typename T>
inline
void appendAttr(std::ostream& stream, const std::string& name, size_t suffix, const T& value)
{
    stream << " " << name << suffix << "=\"" << value << "\"";
}

template <>
inline
void appendAttr<uint8_t>(std::ostream& stream, const std::string& name, const uint8_t& value)
{
    stream << " " << name << "=\"" << static_cast<unsigned>(value) << "\"";
}

template <>
inline
void appendAttr<int8_t>(std::ostream& stream, const std::string& name, const int8_t& value)
{
    stream << " " << name << "=\"" << static_cast<int>(value) << "\"";
}

template <typename T>
inline
void appendResetableTag(std::ostream& stream, const std::string& name, const T& value)
{
    if (!value.empty())
        appendTag(stream, name, value.const_data());
}

template <typename T>
inline
void appendResetableTag(std::ostream& stream, const std::string& name, size_t suffix, const T& value)
{
    if (!value.empty())
        appendTag(stream, name, suffix, value.const_data());
}

template <typename T>
inline
void appendResetableAttr(std::ostream& stream, const std::string& name, const T& value)
{
    if (!value.empty())
        appendAttr(stream, name, value.const_data());
}

template <typename T>
inline
void appendResetableAttr(std::ostream& stream, const std::string& name, size_t suffix, const T& value)
{
    if (!value.empty())
        appendAttr(stream, name, suffix, value.const_data());
}

inline
Optional<std::string> maybeEncode(const Optional<std::string>& value)
{
    Optional<std::string> res;
    if (!value.empty())
        res = Encode12str(value.data());
    return res;
}

inline
Optional<std::string> maybeIconv(const Optional<std::string>& value, const std::string& fromEncoding, const std::string& toEncoding)
{
    Optional<std::string> res;
    if (!value.empty())
        res = IconvString(value.data(), fromEncoding, toEncoding);
    return res;
}

} // namespace STG
