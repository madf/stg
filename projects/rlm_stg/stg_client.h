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

#ifndef STG_CLIENT_H
#define STG_CLIENT_H

#include "stg/os_int.h"

#include <boost/scoped_ptr.hpp>

#include <string>
#include <vector>
#include <stdexcept>

typedef std::vector<std::pair<std::string, std::string> > PAIRS;

struct RESULT
{
    PAIRS modify;
    PAIRS reply;
};

struct ChannelConfig {
    struct Error : std::runtime_error {
        Error(const std::string& message) : runtime_error(message) {}
    };

    ChannelConfig(std::string address);

    std::string transport;
    std::string key;
    std::string address;
    std::string portStr;
    uint16_t port;
};

class STG_CLIENT
{
public:
    enum TYPE {
        AUTHORIZE,
        AUTHENTICATE,
        POST_AUTH,
        PRE_ACCT,
        ACCOUNT
    };
    struct Error : std::runtime_error {
        Error(const std::string& message) : runtime_error(message) {}
    };

    typedef bool (*Callback)(void* /*data*/, const RESULT& /*result*/, bool /*status*/);

    STG_CLIENT(const std::string& address, Callback callback, void* data);
    ~STG_CLIENT();

    bool stop();

    static STG_CLIENT* get();
    static bool configure(const std::string& address, Callback callback, void* data);

    bool request(TYPE type, const std::string& userName, const std::string& password, const PAIRS& pairs);

private:
    class Impl;
    boost::scoped_ptr<Impl> m_impl;
};

#endif
