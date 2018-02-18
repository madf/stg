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
    bool operator==(const PARAM_VALUE & rhs) const
        { return !strcasecmp(param.c_str(), rhs.param.c_str()); }

    bool operator<(const PARAM_VALUE & rhs) const
        { return strcasecmp(param.c_str(), rhs.param.c_str()) < 0; }

    std::string param;
    std::vector<std::string> value;
};
//-----------------------------------------------------------------------------
struct MODULE_SETTINGS
{
    bool operator==(const MODULE_SETTINGS & rhs) const
        { return !strcasecmp(moduleName.c_str(), rhs.moduleName.c_str()); }

    bool operator<(const MODULE_SETTINGS & rhs) const
        { return strcasecmp(moduleName.c_str(), rhs.moduleName.c_str()) < 0; }

    std::string              moduleName;
    std::vector<PARAM_VALUE> moduleParams;
};
//-----------------------------------------------------------------------------
#endif
