#include <sys/utsname.h>

#include "info_methods.h"
#include "version.h"
#include "rpcconfig.h"
#include "common.h"

void METHOD_INFO::execute(xmlrpc_c::paramList const & paramList,
                          xmlrpc_c::value *   const   retvalPtr)
{
paramList.verifyEnd(0);
std::map<std::string, xmlrpc_c::value> structVal;

std::string un;
struct utsname utsn;

uname(&utsn);
un[0] = 0;

un += utsn.sysname;
un += " ";
un += utsn.release;
un += " ";
un += utsn.machine;
un += " ";
un += utsn.nodename;

structVal["version"] = xmlrpc_c::value_string(SERVER_VERSION);
structVal["tariff_num"] = xmlrpc_c::value_int(tariffs->GetTariffsNum());
structVal["tariff"] = xmlrpc_c::value_int(2);
structVal["users_num"] = xmlrpc_c::value_int(users->GetUserNum());
structVal["uname"] = xmlrpc_c::value_string(un);
structVal["dir_num"] = xmlrpc_c::value_int(DIR_NUM);
structVal["day_fee"] = xmlrpc_c::value_int(dayFee);

std::vector<xmlrpc_c::value> dirnameVal;

for (int i = 0; i< DIR_NUM; i++)
    {
    std::string dn2e;
    Encode12str(dn2e, dirNames[i]);
    dirnameVal.push_back(xmlrpc_c::value_string(dn2e));
    }

structVal["dir_names"] = xmlrpc_c::value_array(dirnameVal);

*retvalPtr = xmlrpc_c::value_struct(structVal);
}

void METHOD_LOGIN::execute(xmlrpc_c::paramList const & paramList,
                           xmlrpc_c::value *   const   retvalPtr)
{
std::string login = paramList.getString(0);
std::string password = paramList.getString(1);
paramList.verifyEnd(2);

std::map<std::string, xmlrpc_c::value> structVal;

std::string cookie;
if (config->CheckAdmin(login, password, &cookie))
    {
    structVal["result"] = xmlrpc_c::value_boolean(false);
    structVal["cookie"] = xmlrpc_c::value_string("");
    }
else
    {
    structVal["result"] = xmlrpc_c::value_boolean(true);
    structVal["cookie"] = xmlrpc_c::value_string(cookie);
    }

*retvalPtr = xmlrpc_c::value_struct(structVal);
}

void METHOD_LOGOUT::execute(xmlrpc_c::paramList const & paramList,
                            xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
paramList.verifyEnd(1);

std::map<std::string, xmlrpc_c::value> structVal;

if (config->LogoutAdmin(cookie))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    }
else
    {
    *retvalPtr = xmlrpc_c::value_boolean(true);
    }
}
