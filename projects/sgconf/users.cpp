#include "users.h"

#include "api_action.h"
#include "options.h"
#include "config.h"

#include "stg/servconf.h"
#include "stg/servconf_types.h"
#include "stg/common.h"

#include <iostream>
#include <string>
#include <map>

namespace
{

std::string Indent(size_t level, bool dash = false)
{
if (level == 0)
    return "";
return dash ? std::string(level * 4 - 2, ' ') + "- " : std::string(level * 4, ' ');
}

void PrintUser(const STG::GET_USER::INFO & info, size_t level = 0)
{
std::cout << Indent(level, true) << "login: " << info.login << "\n"
          << Indent(level)       << "password: " << info.password << "\n"
          << Indent(level)       << "cash: " << info.cash << "\n"
          << Indent(level)       << "credit: " << info.credit << "\n"
          << Indent(level)       << "credit expire: " << TimeToString(info.creditExpire) << "\n"
          << Indent(level)       << "last cash add: " << info.lastCashAdd << "\n"
          << Indent(level)       << "last cash add time: " << TimeToString(info.lastCashAddTime) << "\n"
          << Indent(level)       << "prepaid traffic: " << info.prepaidTraff << "\n"
          << Indent(level)       << "disabled: " << (info.disabled ? "t" : "f") << "\n"
          << Indent(level)       << "passive: " << (info.passive ? "t" : "f") << "\n"
          << Indent(level)       << "disabled detail stat: " << (info.disableDetailStat ? "t" : "f") << "\n"
          << Indent(level)       << "connected: " << (info.connected ? "t" : "f") << "\n"
          << Indent(level)       << "always on-line: " << (info.alwaysOnline ? "t" : "f") << "\n"
          << Indent(level)       << "IP: " << inet_ntostring(info.ip) << "\n"
          << Indent(level)       << "IPs: " << info.ips << "\n"
          << Indent(level)       << "tariff: " << info.tariff << "\n"
          << Indent(level)       << "group: " << info.group << "\n"
          << Indent(level)       << "note: " << info.note << "\n"
          << Indent(level)       << "email: " << info.email << "\n"
          << Indent(level)       << "name: " << info.name << "\n"
          << Indent(level)       << "address: " << info.address << "\n"
          << Indent(level)       << "phone: " << info.phone << "\n"
          << Indent(level)       << "free mb: " << info.stat.freeMb << "\n"
          << Indent(level)       << "last ping time: " << TimeToString(info.pingTime) << "\n"
          << Indent(level)       << "last activity time: " << TimeToString(info.lastActivityTime) << "\n"
          << Indent(level)       << "traffic:\n";
for (size_t i = 0; i < DIR_NUM; ++i)
    {
    std::cout << Indent(level + 1, true) << "dir: " << i << "\n"
              << Indent(level + 1)       << "session upload: " << info.stat.su[i] << "\n"
              << Indent(level + 1)       << "session download: " << info.stat.sd[i] << "\n"
              << Indent(level + 1)       << "month upload: " << info.stat.mu[i] << "\n"
              << Indent(level + 1)       << "month download: " << info.stat.md[i] << "\n";
    }
std::cout << Indent(level)       << "user data:\n";
for (size_t i = 0; i < USERDATA_NUM; ++i)
    std::cout << Indent(level + 1, true) << "user data " << i << ": " << info.userData[i] << "\n";
if (!info.authBy.empty())
    {
    std::cout << Indent(level) << "auth by:\n";
    for (size_t i = 0; i < info.authBy.size(); ++i)
        std::cout << Indent(level + 1, true) << info.authBy[i] << "\n";
    }
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

void GetUsersCallback(bool result,
                      const std::string & reason,
                      const std::vector<STG::GET_USER::INFO> & info,
                      void * /*data*/)
{
if (!result)
    {
    std::cerr << "Failed to get user list. Reason: '" << reason << "'." << std::endl;
    return;
    }
std::cout << "Users:\n";
for (size_t i = 0; i < info.size(); ++i)
    PrintUser(info[i], 1);
}

void GetUserCallback(bool result,
                     const std::string & reason,
                     const STG::GET_USER::INFO & info,
                     void * /*data*/)
{
if (!result)
    {
    std::cerr << "Failed to get user. Reason: '" << reason << "'." << std::endl;
    return;
    }
PrintUser(info);
}

bool GetUsersFunction(const SGCONF::CONFIG & config,
                      const std::string & /*arg*/,
                      const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.GetUsers(GetUsersCallback, NULL) == STG::st_ok;
}

bool GetUserFunction(const SGCONF::CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.GetUser(arg, GetUserCallback, NULL) == STG::st_ok;
}

bool DelUserFunction(const SGCONF::CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.DelUser(arg, SimpleCallback, NULL) == STG::st_ok;
}

bool AddUserFunction(const SGCONF::CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & /*options*/)
{
// TODO
std::cerr << "Unimplemented.\n";
return false;
}

bool ChgUserFunction(const SGCONF::CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & options)
{
// TODO
std::cerr << "Unimplemented.\n";
return false;
}

bool CheckUserFunction(const SGCONF::CONFIG & config,
                       const std::string & arg,
                       const std::map<std::string, std::string> & options)
{
// TODO
std::cerr << "Unimplemented.\n";
return false;
}

bool SendMessageFunction(const SGCONF::CONFIG & config,
                         const std::string & arg,
                         const std::map<std::string, std::string> & options)
{
// TODO
std::cerr << "Unimplemented.\n";
return false;
}

} // namespace anonymous

void SGCONF::AppendUsersOptionBlock(COMMANDS & commands, OPTION_BLOCKS & blocks)
{
blocks.Add("User management options")
      .Add("get-users", SGCONF::MakeAPIAction(commands, GetUsersFunction), "\tget user list")
      .Add("get-user", SGCONF::MakeAPIAction(commands, "<login>", true, GetUserFunction), "get user")
      .Add("add-user", SGCONF::MakeAPIAction(commands, "<login>", true, AddUserFunction), "add user")
      .Add("del-user", SGCONF::MakeAPIAction(commands, "<login>", true, DelUserFunction), "del user")
      .Add("chg-user", SGCONF::MakeAPIAction(commands, "<login>", true, ChgUserFunction), "change user")
      .Add("check-user", SGCONF::MakeAPIAction(commands, "<login>", true, CheckUserFunction), "check user existance and credentials")
      .Add("send-message", SGCONF::MakeAPIAction(commands, "<login>", true, SendMessageFunction), "send message");
}
