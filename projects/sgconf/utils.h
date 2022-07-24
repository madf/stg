#pragma once

#include "stg/common.h"

#include <string>
#include <map>
#include <optional>

namespace SGCONF
{

template <typename T>
inline
void MaybeSet(const std::map<std::string, std::string> & options, const std::string & name, std::optional<T> & res)
{
    std::map<std::string, std::string>::const_iterator it(options.find(name));
    if (it == options.end())
        return;
    T value;
    if (str2x(it->second, value) < 0)
        return;
    res = value;
}

template <typename T, typename F>
inline
void MaybeSet(const std::map<std::string, std::string> & options, const std::string & name, T & res, F conv)
{
    std::map<std::string, std::string>::const_iterator it(options.find(name));
    if (it == options.end())
        return;
    conv(it->second, res);
}

template <>
inline
void MaybeSet<std::string>(const std::map<std::string, std::string> & options, const std::string & name, std::optional<std::string> & res)
{
    std::map<std::string, std::string>::const_iterator it(options.find(name));
    if (it == options.end())
        return;
    res = it->second;
}

} // namespace SGCONF
