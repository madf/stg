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
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#ifndef __STG_STGLIBS_SRVCONF_PARSER_GET_USER_H__
#define __STG_STGLIBS_SRVCONF_PARSER_GET_USER_H__

#include "stg/parser.h"

#include "stg/os_int.h"

#include <string>
#include <map>

#include <ctime>

class BASE_PROPERTY_PARSER
{
    public:
        virtual bool Parse(const char ** attr) = 0;
};

template <typename T>
class PROPERTY_PARSER
{
    public:
        typedef T (* FUNC)(const char **);
        PROPERTY_PARSER(T & v, FUNC f) : value(v), func(f) {}
        virtual void Parse(const char ** attr) { value = func(attr); }
    private:
        T & value;
        FUNC func;
};

typedef std::map<std::string, BASE_PROPERTY_PARSER *> PROPERTY_PARSERS;

class PARSER_GET_USER: public PARSER
{
public:
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

    typedef void (* CALLBACK)(const INFO & info, void * data);

    PARSER_GET_USER();
    virtual ~PARSER_GET_USER();
    int  ParseStart(const char *el, const char **attr);
    void ParseEnd(const char *el);
    void SetCallback(CALLBACK f, void * data);
private:
    PROPERTY_PARSERS propertyParsers;
    CALLBACK callback;
    void * data;
    INFO info;
    int depth;

    void ParseUser(const char *el, const char **attr);
    void ParseUserParams(const char *el, const char **attr);
};

#endif
