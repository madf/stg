 /*
 $Revision: 1.5 $
 $Date: 2010/03/04 11:49:52 $
 $Author: faust $
 */

#ifndef BASE_SETTINGS_H
#define BASE_SETTINGS_H

#include <string.h>
#include <string>
#include <vector>

using namespace std;

//-----------------------------------------------------------------------------
struct PARAM_VALUE
{
    PARAM_VALUE()
        : param(),
          value()
    {};
    bool operator==(const PARAM_VALUE & rhs) const
        { return !strcasecmp(param.c_str(), rhs.param.c_str()); };

    bool operator<(const PARAM_VALUE & rhs) const
        { return strcasecmp(param.c_str(), rhs.param.c_str()) < 0; };

    string param;
    vector<string> value;
};
//-----------------------------------------------------------------------------
struct MODULE_SETTINGS
{
    MODULE_SETTINGS()
        : moduleName(),
          moduleParams()
    {};
    MODULE_SETTINGS(const MODULE_SETTINGS & rvalue)
        : moduleName(rvalue.moduleName),
          moduleParams(rvalue.moduleParams)
    {};
    bool operator==(const MODULE_SETTINGS & rhs) const
        { return !strcasecmp(moduleName.c_str(), rhs.moduleName.c_str()); };

    bool operator<(const MODULE_SETTINGS & rhs) const
        { return strcasecmp(moduleName.c_str(), rhs.moduleName.c_str()) < 0; };

string              moduleName;
vector<PARAM_VALUE> moduleParams;
};
//-----------------------------------------------------------------------------
#endif //BASE_SETTINGS_H


