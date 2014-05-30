#include "admins.h"

#include "api_action.h"
#include "options.h"
#include "config.h"
#include "utils.h"

#include "stg/servconf.h"
#include "stg/servconf_types.h"
#include "stg/os_int.h"

#include <iostream>
#include <string>
#include <map>
#include <cassert>

namespace
{

std::string Indent(size_t level, bool dash = false)
{
if (level == 0)
    return "";
return dash ? std::string(level * 4 - 2, ' ') + "- " : std::string(level * 4, ' ');
}

std::string PrivToString(const PRIV& priv)
{
return std::string("") +
       (priv.corpChg ? "1" : "0") +
       (priv.serviceChg ? "1" : "0") +
       (priv.tariffChg ? "1" : "0") +
       (priv.adminChg ? "1" : "0") +
       (priv.userAddDel ? "1" : "0") +
       (priv.userPasswd ? "1" : "0") +
       (priv.userCash ? "1" : "0") +
       (priv.userConf ? "1" : "0") +
       (priv.userStat ? "1" : "0");
}

void PrintAdmin(const STG::GET_ADMIN::INFO & info, size_t level = 0)
{
std::cout << Indent(level, true) << "login: " << info.login << "\n"
          << Indent(level)       << "priviledges: " << PrivToString(info.priv) << "\n";
}

std::vector<SGCONF::API_ACTION::PARAM> GetAdminParams()
{
std::vector<SGCONF::API_ACTION::PARAM> params;
params.push_back(SGCONF::API_ACTION::PARAM("password", "<password>", "password"));
params.push_back(SGCONF::API_ACTION::PARAM("priv", "<priv>", "priviledges"));
return params;
}

void ConvPriv(const std::string & value, RESETABLE<PRIV> & res)
{
if (value.length() != 9)
    throw SGCONF::ACTION::ERROR("Priviledges value should be a 9-digits length binary number.");
PRIV priv;
priv.corpChg = (value[0] == '0' ? 0 : 1);
priv.serviceChg = (value[1] == '0' ? 0 : 1);
priv.tariffChg = (value[2] == '0' ? 0 : 1);
priv.adminChg = (value[3] == '0' ? 0 : 1);
priv.userAddDel = (value[4] == '0' ? 0 : 1);
priv.userPasswd = (value[5] == '0' ? 0 : 1);
priv.userCash = (value[6] == '0' ? 0 : 1);
priv.userConf = (value[7] == '0' ? 0 : 1);
priv.userStat = (value[8] == '0' ? 0 : 1);
res = priv;
}

void SimpleCallback(bool result,
                    const std::string & reason,
                    void * /*data*/)
{
if (!result)
    {
    std::cerr << "Operation failed. Reason: '" << reason << "'." << std::endl;
    return;
    }
std::cout << "Success.\n";
}

void GetAdminsCallback(bool result,
                       const std::string & reason,
                       const std::vector<STG::GET_ADMIN::INFO> & info,
                       void * /*data*/)
{
if (!result)
    {
    std::cerr << "Failed to get admin list. Reason: '" << reason << "'." << std::endl;
    return;
    }
std::cout << "Admins:\n";
for (size_t i = 0; i < info.size(); ++i)
    PrintAdmin(info[i], 1);
}

void GetAdminCallback(bool result,
                      const std::string & reason,
                      const std::vector<STG::GET_ADMIN::INFO> & info,
                      void * data)
{
assert(data != NULL && "Expecting pointer to std::string with the admin's login.");
const std::string & login = *static_cast<const std::string *>(data);
if (!result)
    {
    std::cerr << "Failed to get admin. Reason: '" << reason << "'." << std::endl;
    return;
    }
for (size_t i = 0; i < info.size(); ++i)
    if (info[i].login == login)
        PrintAdmin(info[i]);
}


bool GetAdminsFunction(const SGCONF::CONFIG & config,
                       const std::string & /*arg*/,
                       const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.localAddress.data(),
                    config.localPort.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.GetAdmins(GetAdminsCallback, NULL) == STG::st_ok;
}

bool GetAdminFunction(const SGCONF::CONFIG & config,
                      const std::string & arg,
                      const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.localAddress.data(),
                    config.localPort.data(),
                    config.userName.data(),
                    config.userPass.data());
// STG currently doesn't support <GetAdmin login="..."/>.
// So get a list of admins and filter it. 'data' param holds a pointer to 'login'.
std::string login(arg);
return proto.GetAdmins(GetAdminCallback, &login) == STG::st_ok;
}

bool DelAdminFunction(const SGCONF::CONFIG & config,
                      const std::string & arg,
                      const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.localAddress.data(),
                    config.localPort.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.DelAdmin(arg, SimpleCallback, NULL) == STG::st_ok;
}

bool AddAdminFunction(const SGCONF::CONFIG & config,
                      const std::string & arg,
                      const std::map<std::string, std::string> & options)
{
ADMIN_CONF_RES conf;
conf.login = arg;
SGCONF::MaybeSet(options, "priv", conf.priv, ConvPriv);
SGCONF::MaybeSet(options, "password", conf.password);
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.localAddress.data(),
                    config.localPort.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.AddAdmin(arg, conf, SimpleCallback, NULL) == STG::st_ok;
}

bool ChgAdminFunction(const SGCONF::CONFIG & config,
                      const std::string & arg,
                      const std::map<std::string, std::string> & options)
{
ADMIN_CONF_RES conf;
conf.login = arg;
SGCONF::MaybeSet(options, "priv", conf.priv, ConvPriv);
SGCONF::MaybeSet(options, "password", conf.password);
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.localAddress.data(),
                    config.localPort.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.ChgAdmin(conf, SimpleCallback, NULL) == STG::st_ok;
}

} // namespace anonymous

void SGCONF::AppendAdminsOptionBlock(COMMANDS & commands, OPTION_BLOCKS & blocks)
{
std::vector<API_ACTION::PARAM> params(GetAdminParams());
blocks.Add("Admin management options")
      .Add("get-admins", SGCONF::MakeAPIAction(commands, GetAdminsFunction), "\tget admin list")
      .Add("get-admin", SGCONF::MakeAPIAction(commands, "<login>", GetAdminFunction), "get admin")
      .Add("add-admin", SGCONF::MakeAPIAction(commands, "<login>", params, AddAdminFunction), "add admin")
      .Add("del-admin", SGCONF::MakeAPIAction(commands, "<login>", DelAdminFunction), "del admin")
      .Add("chg-admin", SGCONF::MakeAPIAction(commands, "<login>", params, ChgAdminFunction), "change admin");
}
