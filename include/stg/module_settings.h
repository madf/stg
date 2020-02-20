#pragma once

#include <string>
#include <vector>
#include <cstring> // strcasecmp

namespace STG
{

//-----------------------------------------------------------------------------
struct ParamValue
{
    ParamValue() = default;
    ParamValue(const std::string& p, const std::vector<std::string>& vs) noexcept
        : param(p),
          value(vs)
    {}
    ParamValue(const std::string& p, const std::vector<std::string>& vs, const std::vector<ParamValue>& ss) noexcept
        : param(p),
          value(vs),
          sections(ss)
    {}

    ParamValue(const ParamValue&) = default;
    ParamValue& operator=(const ParamValue&) = default;
    ParamValue(ParamValue&&) = default;
    ParamValue& operator=(ParamValue&&) = default;

    bool operator==(const ParamValue & rhs) const noexcept
    { return !strcasecmp(param.c_str(), rhs.param.c_str()); }

    bool operator<(const ParamValue & rhs) const noexcept
    { return strcasecmp(param.c_str(), rhs.param.c_str()) < 0; }

    std::string param;
    std::vector<std::string> value;
    std::vector<ParamValue> sections;
};
//-----------------------------------------------------------------------------
struct ModuleSettings
{
    ModuleSettings() = default;
    ModuleSettings(const std::string& name, const std::vector<ParamValue>& params) noexcept
        : moduleName(name),
          moduleParams(params)
    {}

    ModuleSettings(const ModuleSettings&) = default;
    ModuleSettings& operator=(const ModuleSettings&) = default;
    ModuleSettings(ModuleSettings&&) = default;
    ModuleSettings& operator=(ModuleSettings&&) = default;

    bool operator==(const ModuleSettings & rhs) const noexcept
    { return !strcasecmp(moduleName.c_str(), rhs.moduleName.c_str()); }

    bool operator<(const ModuleSettings & rhs) const noexcept
    { return strcasecmp(moduleName.c_str(), rhs.moduleName.c_str()) < 0; }

    std::string             moduleName;
    std::vector<ParamValue> moduleParams;
};
//-----------------------------------------------------------------------------

}
