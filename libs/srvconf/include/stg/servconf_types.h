/*
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#pragma once

#include "stg/const.h" // DIR_NUM

#include <string>
#include <vector>
#include <array>
#include <ctime>
#include <cstdint>

namespace STG
{

struct AdminConf;
struct TariffData;
struct ServiceConf;
struct CorpConf;

enum status
{
    st_ok = 0,
    st_conn_fail,
    st_send_fail,
    st_recv_fail,
    st_header_err,
    st_login_err,
    st_logins_err,
    st_data_err,
    st_unknown_err,
    st_dns_err,
    st_xml_parse_error,
    st_data_error
};

namespace Simple
{

using Callback = void (*)(bool result, const std::string& reason, void* data);

} // namespace Simple

namespace GetContainer
{

template <typename T>
struct Callback
{
    using Type = void (*)(bool result, const std::string& reason, const std::vector<T>& info, void* data);
};

} // namespace GetContainer

namespace AuthBy
{

using Info = std::vector<std::string>;
using Callback = void (*)(bool result, const std::string& reason, const Info& info, void* data);

} // namespace AuthBy

namespace ServerInfo
{

struct Info
{
    std::string version;
    int         tariffNum;
    int         tariffType;
    int         usersNum;
    std::string uname;
    int         dirNum;
    std::array<std::string, DIR_NUM> dirName;
};
using Callback = void (*)(bool result, const std::string& reason, const Info& info, void* data);

} // namespace ServerInfo

namespace RawXML
{

using Callback = void (*)(bool result, const std::string& reason, const std::string& response, void* data);

} // namespace RawXML

namespace GetUser
{

struct Stat
{
    std::array<long long, DIR_NUM> su;
    std::array<long long, DIR_NUM> sd;
    std::array<long long, DIR_NUM> mu;
    std::array<long long, DIR_NUM> md;
};

struct Info
{
    std::string login;
    std::string password;
    double      cash;
    double      credit;
    time_t      creditExpire;
    double      lastCashAdd;
    double      lastCashAddTime;
    time_t      lastTimeCash;
    double      prepaidTraff;
    int         disabled;
    int         passive;
    int         disableDetailStat;
    int         connected;
    int         alwaysOnline;
    uint32_t    ip;
    std::string ips;
    std::string tariff;
    std::string group;
    std::string note;
    std::string email;
    std::string name;
    std::string address;
    std::string phone;
    std::string corp;
    Stat        stat;
    time_t      pingTime;
    time_t      lastActivityTime;
    std::array<std::string, USERDATA_NUM> userData;
    std::vector<std::string> services;
    std::vector<std::string> authBy;
};

using Callback = void (*)(bool result, const std::string& reason, const Info& info, void* data);

} // namespace GetUser

namespace GetAdmin
{

using Info = AdminConf;
using Callback = void (*)(bool result, const std::string& reason, const Info& info, void* data);

} // namespace GetAdmin

namespace GetTariff
{

using Info = TariffData;
using Callback = void (*)(bool result, const std::string& reason, const Info& info, void* data);

} // namespace GetTariff

namespace GetService
{

using Info = ServiceConf;
using Callback = void (*)(bool result, const std::string& reason, const Info& info, void* data);

} // namespace GetService

namespace GetCorp
{

using Info = CorpConf;
using Callback = void (*)(bool result, const std::string& reason, const Info& info, void* data);

} // namespace GetCorp

} // namespace STG
