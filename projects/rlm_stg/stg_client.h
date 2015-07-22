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

#include "stg/sgcp_proto.h" // Proto
#include "stg/sgcp_types.h" // TransportType
#include "stg/os_int.h"

#include <boost/thread.hpp>

#include <string>
#include <vector>
#include <utility>

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

    STG::SGCP::TransportType transport;
    std::string key;
    std::string address;
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

    STG_CLIENT(const std::string& address);
    ~STG_CLIENT();

    bool stop();

    static STG_CLIENT* get();
    static bool configure(const std::string& address);

    RESULT request(TYPE type, const std::string& userName, const std::string& password, const PAIRS& pairs);

private:
    ChannelConfig m_config;
    STG::SGCP::Proto m_proto;
    boost::thread m_thread;

    void m_writeHeader(TYPE type, const std::string& userName, const std::string& password);
    void m_writePairBlock(const PAIRS& source);
    PAIRS m_readPairBlock();

    void m_run();
};

#endif
