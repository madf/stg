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

#include "stg/common.h"
#include "stg/optional.h"

#include <string>
#include <cstdint>

namespace SGCONF
{

struct CONFIG
{
    STG::Optional<std::string> configFile;
    STG::Optional<std::string> server;
    STG::Optional<uint16_t> port;
    STG::Optional<std::string> localAddress;
    STG::Optional<uint16_t> localPort;
    STG::Optional<std::string> userName;
    STG::Optional<std::string> userPass;
    STG::Optional<bool> showConfig;

    CONFIG & operator=(const CONFIG & rhs)
    {
    if (!rhs.configFile.empty())
        configFile = rhs.configFile;
    if (!rhs.server.empty())
        server = rhs.server;
    if (!rhs.port.empty())
        port = rhs.port;
    if (!rhs.localAddress.empty())
        localAddress = rhs.localAddress;
    if (!rhs.localPort.empty())
        localPort = rhs.localPort;
    if (!rhs.userName.empty())
        userName = rhs.userName;
    if (!rhs.userPass.empty())
        userPass = rhs.userPass;
    if (!rhs.showConfig.empty())
        showConfig = rhs.showConfig;
    return *this;
    }

    std::string Serialize() const
    {
    std::string res;
    if (!configFile.empty())
        res += "configFile: '" + configFile.data() + "'\n";
    if (!server.empty())
        res += "server: '" + server.data() + "'\n";
    if (!port.empty())
        res += "port: " + std::to_string(port.data()) + "\n";
    if (!localAddress.empty())
        res += "local address: '" + localAddress.data() + "'\n";
    if (!localPort.empty())
        res += "local port: " + std::to_string(localPort.data()) + "\n";
    if (!userName.empty())
        res += "userName: '" + userName.data() + "'\n";
    if (!userPass.empty())
        res += "userPass: '" + userPass.data() + "\n";
    return res;
    }
};

} // namespace SGCONF
