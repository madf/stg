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

#include <map>
#include <string>

namespace STG
{

struct BasePropertyParser
{
    virtual ~BasePropertyParser() = default;
    virtual bool Parse(const char** attr, const std::string& attrName, const std::string& fromEncoding) = 0;
};

template <typename T>
class PropertyParser : public BasePropertyParser
{
    public:
        using Func = bool (*)(const char **, T&, const std::string&);
        PropertyParser(T& v, Func f) : value(v), func(f) {}
        PropertyParser(T& v, Func f, const std::string& e) : value(v), func(f), encoding(e) {}
        bool Parse(const char** attr, const std::string& attrName, const std::string& /*fromEncoding*/) override { return func(attr, value, attrName); }
    private:
        T & value;
        Func func;
        std::string encoding;
};

template <>
inline
bool PropertyParser<std::string>::Parse(const char** attr, const std::string& attrName, const std::string& toEncoding)
{
    if (!encoding.empty() && !toEncoding.empty())
    {
        std::string tmp;
        if (!func(attr, tmp, attrName))
            return false;
        value = IconvString(tmp, encoding, toEncoding);
        return true;
    }

    return func(attr, value, attrName);
}

using PropertyParsers = std::map<std::string, BasePropertyParser *>;

bool checkValue(const char** attr, const std::string& attrName);

template <typename T>
inline
bool getValue(const char** attr, T& value, const std::string& attrName)
{
    if (checkValue(attr, attrName))
        if (str2x(attr[1], value) < 0)
            return false;
    return true;
}

template <>
inline
bool getValue<std::string>(const char** attr, std::string& value, const std::string& attrName)
{
    if (!checkValue(attr, attrName))
        return false;
    value = attr[1];
    return true;
}

template <>
inline
bool getValue<double>(const char** attr, double& value, const std::string& attrName)
{
    if (checkValue(attr, attrName))
        if (strtodouble2(attr[1], value))
            return false;
    return true;
}

bool getEncodedValue(const char** attr, std::string& value, const std::string& attrName);

bool getIPValue(const char** attr, uint32_t& value, const std::string& attrName);

template <typename T>
inline
void addParser(PropertyParsers& parsers, const std::string& name, T& value, const typename PropertyParser<T>::Func& func = getValue<T>)
{
    parsers.insert(std::make_pair(ToLower(name), new PropertyParser<T>(value, func)));
}

template <typename T>
inline
void addParser(PropertyParsers& parsers, const std::string& name, T& value, const std::string& toEncoding, const typename PropertyParser<T>::Func& func = getValue<T>)
{
    parsers.insert(std::make_pair(ToLower(name), new PropertyParser<T>(value, func, toEncoding)));
}

bool tryParse(PropertyParsers& parsers, const std::string& name, const char** attr, const std::string& fromEncoding, const std::string& attrName = "value");

} // namespace STG
