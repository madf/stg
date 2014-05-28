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

std::vector<SGCONF::API_ACTION::PARAM> GetUserParams()
{
std::vector<SGCONF::API_ACTION::PARAM> params;
params.push_back({"password", "<password>", "\tuser's password"});
params.push_back({"cash", "<cash>", "\t\tuser's cash"});
params.push_back({"credit", "<amount>", "\tuser's credit"});
params.push_back({"credit-expire", "<date>", "\tcredit expiration"});
params.push_back({"free", "<free mb>", "\tprepaid traffic"});
params.push_back({"disabled", "<flag>", "\tdisable user (y|n)"});
params.push_back({"passive", "<flag>", "\tmake user passive (y|n)"});
params.push_back({"disable-detail-stat", "<flag>", "disable detail stat (y|n)"});
params.push_back({"always-online", "<flag>", "\tmake user always online (y|n)"});
params.push_back({"ips", "<ips>", "\t\tcoma-separated list of ips"});
params.push_back({"tariff", "<tariff name>", "\tcurrent tariff"});
params.push_back({"next-tariff", "<tariff name>", "tariff starting from the next month"});
params.push_back({"group", "<group>", "\t\tuser's group"});
params.push_back({"note", "<note>", "\t\tuser's note"});
params.push_back({"email", "<email>", "\t\tuser's email"});
params.push_back({"name", "<real name>", "\tuser's real name"});
params.push_back({"address", "<address>", "\tuser's postal address"});
params.push_back({"phone", "<phone>", "\t\tuser's phone number"});
params.push_back({"session-traffic", "<up/dn, ...>", "coma-separated session upload and download"});
params.push_back({"month-traffic", "<up/dn, ...>", "coma-separated month upload and download"});
params.push_back({"user-data", "<value, ...>", "coma-separated user data values"});
return params;
}

std::vector<SGCONF::API_ACTION::PARAM> GetCheckParams()
{
std::vector<SGCONF::API_ACTION::PARAM> params;
params.push_back({"password", "<password>", "\tuser's password"});
return params;
}

std::vector<SGCONF::API_ACTION::PARAM> GetMessageParams()
{
std::vector<SGCONF::API_ACTION::PARAM> params;
params.push_back({"logins", "<login, ...>", "\tlist of logins to send a message"});
params.push_back({"text", "<text>", "\t\tmessage text"});
return params;
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
std::vector<API_ACTION::PARAM> params(GetUserParams());
blocks.Add("User management options")
      .Add("get-users", SGCONF::MakeAPIAction(commands, GetUsersFunction), "\tget user list")
      .Add("get-user", SGCONF::MakeAPIAction(commands, "<login>", GetUserFunction), "get user")
      .Add("add-user", SGCONF::MakeAPIAction(commands, "<login>", params, AddUserFunction), "add user")
      .Add("del-user", SGCONF::MakeAPIAction(commands, "<login>", DelUserFunction), "del user")
      .Add("chg-user", SGCONF::MakeAPIAction(commands, "<login>", params, ChgUserFunction), "change user")
      .Add("check-user", SGCONF::MakeAPIAction(commands, "<login>", GetCheckParams(), CheckUserFunction), "check user existance and credentials")
      .Add("send-message", SGCONF::MakeAPIAction(commands, GetMessageParams(), SendMessageFunction), "send message");
}
