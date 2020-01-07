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

#ifndef __STG_STGLIBS_SRVCONF_PROPERTY_PARSERS_H__
#define __STG_STGLIBS_SRVCONF_PROPERTY_PARSERS_H__

#include "stg/common.h"

#include <map>
#include <string>

namespace STG
{

class BASE_PROPERTY_PARSER
{
    public:
        virtual ~BASE_PROPERTY_PARSER() {}
        virtual bool Parse(const char ** attr, const std::string & attrName, const std::string & fromEncoding) = 0;
};

template <typename T>
class PROPERTY_PARSER : public BASE_PROPERTY_PARSER
{
    public:
        typedef bool (* FUNC)(const char **, T &, const std::string &);
        PROPERTY_PARSER(T & v, FUNC f) : value(v), func(f) {}
        PROPERTY_PARSER(T & v, FUNC f, const std::string & e) : value(v), func(f), encoding(e) {}
        virtual bool Parse(const char ** attr, const std::string & attrName, const std::string & /*fromEncoding*/) { return func(attr, value, attrName); }
    private:
        T & value;
        FUNC func;
        std::string encoding;
};

template <>
inline
bool PROPERTY_PARSER<std::string>::Parse(const char ** attr, const std::string & attrName, const std::string & toEncoding)
{
if (!encoding.empty() && !toEncoding.empty())
    {
    std::string tmp;
    if (!func(attr, tmp, attrName))
        return false;
    value = IconvString(tmp, encoding, toEncoding);
    return true;
    }
else
    return func(attr, value, attrName);
}

typedef std::map<std::string, BASE_PROPERTY_PARSER *> PROPERTY_PARSERS;

bool CheckValue(const char ** attr, const std::string & attrName);

template <typename T>
inline
bool GetValue(const char ** attr, T & value, const std::string & attrName)
{
if (CheckValue(attr, attrName))
    if (str2x(attr[1], value) < 0)
        return false;
return true;
}

template <>
inline
bool GetValue<std::string>(const char ** attr, std::string & value, const std::string & attrName)
{
if (!CheckValue(attr, attrName))
    return false;
value = attr[1];
return true;
}

template <>
inline
bool GetValue<double>(const char ** attr, double & value, const std::string & attrName)
{
if (CheckValue(attr, attrName))
    if (strtodouble2(attr[1], value))
        return false;
return true;
}

bool GetEncodedValue(const char ** attr, std::string & value, const std::string & attrName);

bool GetIPValue(const char ** attr, uint32_t& value, const std::string & attrName);

template <typename T>
inline
void AddParser(PROPERTY_PARSERS & parsers, const std::string & name, T & value, const typename PROPERTY_PARSER<T>::FUNC & func = GetValue<T>)
{
    parsers.insert(std::make_pair(ToLower(name), new PROPERTY_PARSER<T>(value, func)));
}

template <typename T>
inline
void AddParser(PROPERTY_PARSERS & parsers, const std::string & name, T & value, const std::string & toEncoding, const typename PROPERTY_PARSER<T>::FUNC & func = GetValue<T>)
{
    parsers.insert(std::make_pair(ToLower(name), new PROPERTY_PARSER<T>(value, func, toEncoding)));
}

bool TryParse(PROPERTY_PARSERS & parsers, const std::string & name, const char ** attr, const std::string & fromEncoding, const std::string & attrName = "value");

} // namespace STG

#endif
