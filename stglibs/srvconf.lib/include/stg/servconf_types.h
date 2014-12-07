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

#ifndef __STG_STGLIBS_SRVCONF_TYPES_H__
#define __STG_STGLIBS_SRVCONF_TYPES_H__

#include "stg/array.h"
#include "stg/const.h" // DIR_NUM
#include "stg/os_int.h" // uint32_t, etc...

#include <string>
#include <vector>
#include <ctime>

struct ADMIN_CONF;
struct TARIFF_DATA;
struct SERVICE_CONF;
struct CORP_CONF;

namespace STG
{

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

namespace SIMPLE
{

typedef void (* CALLBACK)(bool result, const std::string & reason, void * data);

} // namespace SIMPLE

namespace GET_CONTAINER
{

template <typename INFO>
struct CALLBACK
{
typedef void (* TYPE)(bool result, const std::string & reason, const std::vector<INFO> & info, void * data);
};

} // namespace GET_CONTAINER

namespace AUTH_BY
{

typedef std::vector<std::string> INFO;
typedef void (* CALLBACK)(bool result, const std::string & reason, const INFO & info, void * data);

} // namespace AUTH_BY

namespace SERVER_INFO
{

struct INFO
{
    std::string version;
    int         tariffNum;
    int         tariffType;
    int         usersNum;
    std::string uname;
    int         dirNum;
    ARRAY<std::string, DIR_NUM> dirName;
};
typedef void (* CALLBACK)(bool result, const std::string & reason, const INFO & info, void * data);

} // namespace SERVER_INFO

namespace RAW_XML
{

typedef void (* CALLBACK)(bool result, const std::string & reason, const std::string & response, void * data);

} // namespace RAW_XML

namespace GET_USER
{

struct STAT
{
    ARRAY<long long, DIR_NUM> su;
    ARRAY<long long, DIR_NUM> sd;
    ARRAY<long long, DIR_NUM> mu;
    ARRAY<long long, DIR_NUM> md;
};

struct INFO
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
    STAT        stat;
    time_t      pingTime;
    time_t      lastActivityTime;
    ARRAY<std::string, USERDATA_NUM> userData;
    std::vector<std::string> services;
    std::vector<std::string> authBy;
};

typedef void (* CALLBACK)(bool result, const std::string & reason, const INFO & info, void * data);

} // namespace GET_USER

namespace GET_ADMIN
{

typedef ADMIN_CONF INFO;
typedef void (* CALLBACK)(bool result, const std::string & reason, const INFO & info, void * data);

} // namespace GET_ADMIN

namespace GET_TARIFF
{

typedef TARIFF_DATA INFO;
typedef void (* CALLBACK)(bool result, const std::string & reason, const INFO & info, void * data);

} // namespace GET_TARIFF

namespace GET_SERVICE
{

typedef SERVICE_CONF INFO;
typedef void (* CALLBACK)(bool result, const std::string & reason, const INFO & info, void * data);

} // namespace GET_SERVICE

namespace GET_CORP
{

typedef CORP_CONF INFO;
typedef void (* CALLBACK)(bool result, const std::string & reason, const INFO & info, void * data);

} // namespace GET_CORP

} // namespace STG

#endif
