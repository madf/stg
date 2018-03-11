 /*
 $Revision: 1.5 $
 $Date: 2010/03/04 11:49:52 $
 $Author: faust $
 */

#ifndef MODULE_SETTINGS_H
#define MODULE_SETTINGS_H

#include <cstring> // strcasecmp
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
struct PARAM_VALUE
{
    PARAM_VALUE() {}
    PARAM_VALUE(const std::string& p, const std::vector<std::string>& vs)
        : param(p),
          value(vs)
    {}
    PARAM_VALUE(const std::string& p, const std::vector<std::string>& vs, const std::vector<PARAM_VALUE>& ss)
        : param(p),
          value(vs),
          sections(ss)
    {}
    bool operator==(const PARAM_VALUE & rhs) const
        { return !strcasecmp(param.c_str(), rhs.param.c_str()); }

    bool operator<(const PARAM_VALUE & rhs) const
        { return strcasecmp(param.c_str(), rhs.param.c_str()) < 0; }

    std::string param;
    std::vector<std::string> value;
    std::vector<PARAM_VALUE> sections;
};
//-----------------------------------------------------------------------------
struct MODULE_SETTINGS
{
    MODULE_SETTINGS() {}
    MODULE_SETTINGS(const std::string& name, const std::vector<PARAM_VALUE>& params)
        : moduleName(name),
          moduleParams(params)
    {}
    bool operator==(const MODULE_SETTINGS & rhs) const
        { return !strcasecmp(moduleName.c_str(), rhs.moduleName.c_str()); }

    bool operator<(const MODULE_SETTINGS & rhs) const
        { return strcasecmp(moduleName.c_str(), rhs.moduleName.c_str()) < 0; }

    std::string              moduleName;
    std::vector<PARAM_VALUE> moduleParams;
};
//-----------------------------------------------------------------------------
#endif
