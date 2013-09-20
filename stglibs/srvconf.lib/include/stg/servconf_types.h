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

#include "stg/const.h" // DIR_NUM
#include "stg/os_int.h" // uint32_t, etc...

#include <string>
#include <vector>

#define  STG_HEADER     "SG04"
#define  OK_HEADER      "OKHD"
#define  ERR_HEADER     "ERHD"
#define  OK_LOGIN       "OKLG"
#define  ERR_LOGIN      "ERLG"
#define  OK_LOGINS      "OKLS"
#define  ERR_LOGINS     "ERLS"

#define  ENC_MSG_LEN    (8)

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

enum CONF_STATE
{
confHdr = 0,
confLogin,
confLoginCipher,
confData
};

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
    std::string dirName[DIR_NUM];
};
typedef void (* CALLBACK)(bool result, const std::string & reason, const INFO & info, void * data);

} // namespace SERVER_INFO

namespace CHECK_USER
{

typedef int (* CALLBACK)(bool result, const std::string & reason, void * data);

} // namespace CHECK_USER

namespace GET_USER
{

struct STAT
{
    long long  su[DIR_NUM];
    long long  sd[DIR_NUM];
    long long  mu[DIR_NUM];
    long long  md[DIR_NUM];
    double     freeMb;
};

struct INFO
{
    std::string login;
    std::string password;
    double      cash;
    double      credit;
    time_t      creditExpire;
    double      lastCash;
    double      prepaidTraff;
    int         down;
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
    STAT        stat;
    std::string userData[USERDATA_NUM];
};

typedef void (* CALLBACK)(bool result, const std::string & reason, const INFO & info, void * data);

} // namespace GET_USER

namespace GET_USERS
{

typedef std::vector<GET_USER::INFO> INFO;
typedef void (* CALLBACK)(bool result, const std::string & reason, const INFO & info, void * data);

} // namespace GET_USERS

namespace CHG_USER
{

typedef void (* CALLBACK)(bool result, const std::string & reason, void * data);

}

namespace SEND_MESSAGE
{

typedef void (* CALLBACK)(bool result, const std::string & reason, void * data);

}

} // namespace STG

#endif
