#include "users.h"

#include "api_action.h"
#include "options.h"
#include "config.h"
#include "utils.h"

#include "stg/servconf.h"
#include "stg/servconf_types.h"
#include "stg/user_conf.h"
#include "stg/user_stat.h"
#include "stg/user_ips.h"
#include "stg/common.h"

#include <iostream>
#include <algorithm>
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
          << Indent(level)       << "corporation: " << info.corp << "\n"
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
if (!info.services.empty())
    {
    std::cout << Indent(level) << "services:\n";
    for (size_t i = 0; i < info.services.size(); ++i)
        std::cout << Indent(level + 1, true) << info.services[i] << "\n";
    }
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
params.push_back(SGCONF::API_ACTION::PARAM("password", "<password>", "\tuser's password"));
params.push_back(SGCONF::API_ACTION::PARAM("cash-add", "<cash[:message]>", "cash to add (with optional comment)"));
params.push_back(SGCONF::API_ACTION::PARAM("cash-set", "<cash[:message]>", "cash to set (with optional comment)"));
params.push_back(SGCONF::API_ACTION::PARAM("credit", "<amount>", "\tuser's credit"));
params.push_back(SGCONF::API_ACTION::PARAM("credit-expire", "<date>", "\tcredit expiration"));
params.push_back(SGCONF::API_ACTION::PARAM("free", "<free mb>", "\tprepaid traffic"));
params.push_back(SGCONF::API_ACTION::PARAM("disabled", "<flag>", "\tdisable user (y|n)"));
params.push_back(SGCONF::API_ACTION::PARAM("passive", "<flag>", "\tmake user passive (y|n)"));
params.push_back(SGCONF::API_ACTION::PARAM("disable-detail-stat", "<flag>", "disable detail stat (y|n)"));
params.push_back(SGCONF::API_ACTION::PARAM("always-online", "<flag>", "\tmake user always online (y|n)"));
params.push_back(SGCONF::API_ACTION::PARAM("ips", "<ips>", "\t\tcoma-separated list of ips"));
params.push_back(SGCONF::API_ACTION::PARAM("tariff", "<tariff name>", "\tcurrent tariff"));
params.push_back(SGCONF::API_ACTION::PARAM("next-tariff", "<tariff name>", "tariff starting from the next month"));
params.push_back(SGCONF::API_ACTION::PARAM("group", "<group>", "\t\tuser's group"));
params.push_back(SGCONF::API_ACTION::PARAM("note", "<note>", "\t\tuser's note"));
params.push_back(SGCONF::API_ACTION::PARAM("email", "<email>", "\t\tuser's email"));
params.push_back(SGCONF::API_ACTION::PARAM("name", "<real name>", "\tuser's real name"));
params.push_back(SGCONF::API_ACTION::PARAM("address", "<address>", "\tuser's postal address"));
params.push_back(SGCONF::API_ACTION::PARAM("phone", "<phone>", "\t\tuser's phone number"));
params.push_back(SGCONF::API_ACTION::PARAM("corp", "<corp name>", "\tcorporation name"));
params.push_back(SGCONF::API_ACTION::PARAM("session-traffic", "<up/dn, ...>", "coma-separated session upload and download"));
params.push_back(SGCONF::API_ACTION::PARAM("month-traffic", "<up/dn, ...>", "coma-separated month upload and download"));
params.push_back(SGCONF::API_ACTION::PARAM("user-data", "<value, ...>", "coma-separated user data values"));
return params;
}

std::vector<SGCONF::API_ACTION::PARAM> GetCheckParams()
{
std::vector<SGCONF::API_ACTION::PARAM> params;
params.push_back(SGCONF::API_ACTION::PARAM("password", "<password>", "\tuser's password"));
return params;
}

std::vector<SGCONF::API_ACTION::PARAM> GetMessageParams()
{
std::vector<SGCONF::API_ACTION::PARAM> params;
params.push_back(SGCONF::API_ACTION::PARAM("logins", "<login, ...>", "\tlist of logins to send a message"));
params.push_back(SGCONF::API_ACTION::PARAM("text", "<text>", "\t\tmessage text"));
return params;
}

void ConvBool(const std::string & value, RESETABLE<int> & res)
{
res = !value.empty() && value[0] == 'y';
}

void Splice(std::vector<RESETABLE<std::string> > & lhs, const std::vector<RESETABLE<std::string> > & rhs)
{
for (size_t i = 0; i < lhs.size() && i < rhs.size(); ++i)
    lhs[i].splice(rhs[i]);
}

RESETABLE<std::string> ConvString(const std::string & value)
{
return value;
}

void ConvStringList(std::string value, std::vector<RESETABLE<std::string> > & res)
{
Splice(res, Split<std::vector<RESETABLE<std::string> > >(value, ',', ConvString));
}

void ConvServices(std::string value, RESETABLE<std::vector<std::string> > & res)
{
value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
res = Split<std::vector<std::string> >(value, ',');
}

void ConvCreditExpire(const std::string & value, RESETABLE<time_t> & res)
{
struct tm brokenTime;
if (stg_strptime(value.c_str(), "%Y-%m-%d %H:%M:%S", &brokenTime) == NULL)
    throw SGCONF::ACTION::ERROR("Credit expiration should be in format 'YYYY-MM-DD HH:MM:SS'. Got: '" + value + "'");
res = stg_timegm(&brokenTime);
}

void ConvIPs(const std::string & value, RESETABLE<USER_IPS> & res)
{
res = StrToIPS(value);
}

struct TRAFF
{
    uint64_t up;
    uint64_t down;
};

TRAFF ConvTraff(const std::string & value)
{
TRAFF res;
size_t slashPos = value.find_first_of('/');
if (slashPos == std::string::npos)
    throw SGCONF::ACTION::ERROR("Traffic record should be in format 'upload/download'. Got: '" + value + "'");

if (str2x(value.substr(0, slashPos), res.up) < 0)
    throw SGCONF::ACTION::ERROR("Traffic value should be an integer. Got: '" + value.substr(0, slashPos) + "'");
if (str2x(value.substr(slashPos + 1, value.length() - slashPos), res.down) < 0)
    throw SGCONF::ACTION::ERROR("Traffic value should be an integer. Got: '" + value.substr(slashPos + 1, value.length() - slashPos) + "'");
return res;
}

void ConvSessionTraff(std::string value, USER_STAT_RES & res)
{
value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
std::vector<TRAFF> traff(Split<std::vector<TRAFF> >(value, ',', ConvTraff));
if (traff.size() != DIR_NUM)
    throw SGCONF::ACTION::ERROR("There should be prcisely " + x2str(DIR_NUM) + " records of session traffic.");
for (size_t i = 0; i < DIR_NUM; ++i)
    {
    res.sessionUp[i] = traff[i].up;
    res.sessionDown[i] = traff[i].down;
    }
}

void ConvMonthTraff(std::string value, USER_STAT_RES & res)
{
value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
std::vector<TRAFF> traff(Split<std::vector<TRAFF> >(value, ',', ConvTraff));
if (traff.size() != DIR_NUM)
    throw SGCONF::ACTION::ERROR("There should be prcisely " + x2str(DIR_NUM) + " records of month traffic.");
for (size_t i = 0; i < DIR_NUM; ++i)
    {
    res.monthUp[i] = traff[i].up;
    res.monthDown[i] = traff[i].down;
    }
}

void ConvCashInfo(const std::string & value, RESETABLE<CASH_INFO> & res)
{
CASH_INFO info;
size_t pos = value.find_first_of(':');
if (pos == std::string::npos)
    {
    if (str2x(value, info.first) < 0)
        throw SGCONF::ACTION::ERROR("Cash should be a double value. Got: '" + value + "'");
    }
else
    {
    if (str2x(value.substr(0, pos), info.first) < 0)
        throw SGCONF::ACTION::ERROR("Cash should be a double value. Got: '" + value + "'");
    info.second = value.substr(pos + 1);
    }
res = info;
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

void AuthByCallback(bool result,
                    const std::string & reason,
                    const std::vector<std::string> & info,
                    void * /*data*/)
{
if (!result)
    {
    std::cerr << "Failed to get authorizer list. Reason: '" << reason << "'." << std::endl;
    return;
    }
std::cout << "Authorized by:\n";
for (size_t i = 0; i < info.size(); ++i)
    std::cout << Indent(1, true) << info[i] << "\n";
}

bool GetUsersFunction(const SGCONF::CONFIG & config,
                      const std::string & /*arg*/,
                      const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.localAddress.data(),
                    config.localPort.data(),
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
                    config.localAddress.data(),
                    config.localPort.data(),
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
                    config.localAddress.data(),
                    config.localPort.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.DelUser(arg, SimpleCallback, NULL) == STG::st_ok;
}

bool AddUserFunction(const SGCONF::CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & options)
{
USER_CONF_RES conf;
SGCONF::MaybeSet(options, "password", conf.password);
SGCONF::MaybeSet(options, "passive", conf.passive, ConvBool);
SGCONF::MaybeSet(options, "disabled", conf.disabled, ConvBool);
SGCONF::MaybeSet(options, "disable-detail-stat", conf.disabledDetailStat, ConvBool);
SGCONF::MaybeSet(options, "always-online", conf.alwaysOnline, ConvBool);
SGCONF::MaybeSet(options, "tariff", conf.tariffName);
SGCONF::MaybeSet(options, "address", conf.address);
SGCONF::MaybeSet(options, "phone", conf.phone);
SGCONF::MaybeSet(options, "email", conf.email);
SGCONF::MaybeSet(options, "note", conf.note);
SGCONF::MaybeSet(options, "name", conf.realName);
SGCONF::MaybeSet(options, "corp", conf.corp);
SGCONF::MaybeSet(options, "services", conf.services, ConvServices);
SGCONF::MaybeSet(options, "group", conf.group);
SGCONF::MaybeSet(options, "credit", conf.credit);
SGCONF::MaybeSet(options, "next-tariff", conf.nextTariff);
SGCONF::MaybeSet(options, "user-data", conf.userdata, ConvStringList);
SGCONF::MaybeSet(options, "credit-expire", conf.creditExpire, ConvCreditExpire);
SGCONF::MaybeSet(options, "ips", conf.ips, ConvIPs);
USER_STAT_RES stat;
SGCONF::MaybeSet(options, "cash-set", stat.cashSet, ConvCashInfo);
SGCONF::MaybeSet(options, "free", stat.freeMb);
SGCONF::MaybeSet(options, "session-traffic", stat, ConvSessionTraff);
SGCONF::MaybeSet(options, "month-traffic", stat, ConvMonthTraff);
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.localAddress.data(),
                    config.localPort.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.AddUser(arg, conf, stat, SimpleCallback, NULL) == STG::st_ok;
}

bool ChgUserFunction(const SGCONF::CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & options)
{
USER_CONF_RES conf;
SGCONF::MaybeSet(options, "password", conf.password);
SGCONF::MaybeSet(options, "passive", conf.passive, ConvBool);
SGCONF::MaybeSet(options, "disabled", conf.disabled, ConvBool);
SGCONF::MaybeSet(options, "disable-detail-stat", conf.disabledDetailStat, ConvBool);
SGCONF::MaybeSet(options, "always-online", conf.alwaysOnline, ConvBool);
SGCONF::MaybeSet(options, "tariff", conf.tariffName);
SGCONF::MaybeSet(options, "address", conf.address);
SGCONF::MaybeSet(options, "phone", conf.phone);
SGCONF::MaybeSet(options, "email", conf.email);
SGCONF::MaybeSet(options, "note", conf.note);
SGCONF::MaybeSet(options, "name", conf.realName);
SGCONF::MaybeSet(options, "corp", conf.corp);
SGCONF::MaybeSet(options, "services", conf.services, ConvServices);
SGCONF::MaybeSet(options, "group", conf.group);
SGCONF::MaybeSet(options, "credit", conf.credit);
SGCONF::MaybeSet(options, "next-tariff", conf.nextTariff);
SGCONF::MaybeSet(options, "user-data", conf.userdata, ConvStringList);
SGCONF::MaybeSet(options, "credit-expire", conf.creditExpire, ConvCreditExpire);
SGCONF::MaybeSet(options, "ips", conf.ips, ConvIPs);
USER_STAT_RES stat;
SGCONF::MaybeSet(options, "cash-add", stat.cashAdd, ConvCashInfo);
SGCONF::MaybeSet(options, "cash-set", stat.cashSet, ConvCashInfo);
SGCONF::MaybeSet(options, "free", stat.freeMb);
SGCONF::MaybeSet(options, "session-traffic", stat, ConvSessionTraff);
SGCONF::MaybeSet(options, "month-traffic", stat, ConvMonthTraff);
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.localAddress.data(),
                    config.localPort.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.ChgUser(arg, conf, stat, SimpleCallback, NULL) == STG::st_ok;
}

bool CheckUserFunction(const SGCONF::CONFIG & config,
                       const std::string & arg,
                       const std::map<std::string, std::string> & options)
{
std::map<std::string, std::string>::const_iterator it(options.find("password"));
if (it == options.end())
    throw SGCONF::ACTION::ERROR("Password is not specified.");
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.localAddress.data(),
                    config.localPort.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.CheckUser(arg, it->second, SimpleCallback, NULL) == STG::st_ok;
}

bool SendMessageFunction(const SGCONF::CONFIG & config,
                         const std::string & /*arg*/,
                         const std::map<std::string, std::string> & options)
{
std::map<std::string, std::string>::const_iterator it(options.find("logins"));
if (it == options.end())
    throw SGCONF::ACTION::ERROR("Logins are not specified.");
std::string logins = it->second;
for (size_t i = 0; i < logins.length(); ++i)
    if (logins[i] == ',')
        logins[i] = ':';
it = options.find("text");
if (it == options.end())
    throw SGCONF::ACTION::ERROR("Message text is not specified.");
std::string text = it->second;
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.localAddress.data(),
                    config.localPort.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.SendMessage(logins, text, SimpleCallback, NULL) == STG::st_ok;
}

bool AuthByFunction(const SGCONF::CONFIG & config,
                    const std::string & arg,
                    const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.localAddress.data(),
                    config.localPort.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.AuthBy(arg, AuthByCallback, NULL) == STG::st_ok;
}

} // namespace anonymous

void SGCONF::AppendUsersOptionBlock(COMMANDS & commands, OPTION_BLOCKS & blocks)
{
std::vector<API_ACTION::PARAM> params(GetUserParams());
blocks.Add("User management options")
      .Add("get-users", SGCONF::MakeAPIAction(commands, GetUsersFunction), "\tget user list")
      .Add("get-user", SGCONF::MakeAPIAction(commands, "<login>", GetUserFunction), "get user")
      .Add("add-user", SGCONF::MakeAPIAction(commands, "<login>", params, AddUserFunction), "add user")
      .Add("del-user", SGCONF::MakeAPIAction(commands, "<login>", DelUserFunction), "delete user")
      .Add("chg-user", SGCONF::MakeAPIAction(commands, "<login>", params, ChgUserFunction), "change user")
      .Add("check-user", SGCONF::MakeAPIAction(commands, "<login>", GetCheckParams(), CheckUserFunction), "check user existance and credentials")
      .Add("send-message", SGCONF::MakeAPIAction(commands, GetMessageParams(), SendMessageFunction), "send message")
      .Add("auth-by", SGCONF::MakeAPIAction(commands, "<login>", AuthByFunction), "a list of authorizers user authorized by");
}
