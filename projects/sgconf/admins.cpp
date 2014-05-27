#include "admins.h"

#include "config.h"

#include "stg/servconf.h"
#include "stg/servconf_types.h"
#include "stg/os_int.h"

#include <iostream>
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

} // namespace anonymous


bool SGCONF::GetAdminsFunction(const SGCONF::CONFIG & config,
                               const std::string & /*arg*/,
                               const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.GetAdmins(GetAdminsCallback, NULL) == STG::st_ok;
}

bool SGCONF::GetAdminFunction(const SGCONF::CONFIG & config,
                              const std::string & arg,
                              const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.userName.data(),
                    config.userPass.data());
// STG currently doesn't support <GetAdmin login="..."/>.
// So get a list of admins and filter it. 'data' param holds a pointer to 'login'.
std::string login(arg);
return proto.GetAdmins(GetAdminCallback, &login) == STG::st_ok;
}

bool SGCONF::DelAdminFunction(const SGCONF::CONFIG & config,
                              const std::string & arg,
                              const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.DelAdmin(arg, SimpleCallback, NULL) == STG::st_ok;
}

bool SGCONF::AddAdminFunction(const SGCONF::CONFIG & config,
                              const std::string & arg,
                              const std::map<std::string, std::string> & /*options*/)
{
// TODO
std::cerr << "Unimplemented.\n";
return false;
}

bool SGCONF::ChgAdminFunction(const SGCONF::CONFIG & config,
                              const std::string & arg,
                              const std::map<std::string, std::string> & options)
{
// TODO
std::cerr << "Unimplemented.\n";
return false;
}
