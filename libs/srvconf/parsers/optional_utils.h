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

#include "stg/common.h"

#include <string>
#include <ostream>
#include <optional>

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
void appendResetableTag(std::ostream& stream, const std::string& name, const std::optional<T>& value)
{
    if (value)
        appendTag(stream, name, value.value());
}

template <typename T>
inline
void appendResetableTag(std::ostream& stream, const std::string& name, size_t suffix, const std::optional<T>& value)
{
    if (value)
        appendTag(stream, name, suffix, value.value());
}

template <typename T>
inline
void appendResetableAttr(std::ostream& stream, const std::string& name, const std::optional<T>& value)
{
    if (value)
        appendAttr(stream, name, value.value());
}

template <typename T>
inline
void appendResetableAttr(std::ostream& stream, const std::string& name, size_t suffix, const std::optional<T>& value)
{
    if (value)
        appendAttr(stream, name, suffix, value.value());
}

inline
std::optional<std::string> maybeEncode(const std::optional<std::string>& value)
{
    std::optional<std::string> res;
    if (value)
        res = Encode12str(value.value());
    return res;
}

inline
std::optional<std::string> maybeIconv(const std::optional<std::string>& value, const std::string& fromEncoding, const std::string& toEncoding)
{
    std::optional<std::string> res;
    if (value)
        res = IconvString(value.value(), fromEncoding, toEncoding);
    return res;
}

} // namespace STG
