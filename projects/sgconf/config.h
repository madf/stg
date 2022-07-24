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

#include "stg/splice.h"
#include "stg/common.h"

#include <string>
#include <optional>
#include <cstdint>

namespace SGCONF
{

struct CONFIG
{
    std::optional<std::string> configFile;
    std::optional<std::string> server;
    std::optional<uint16_t> port;
    std::optional<std::string> localAddress;
    std::optional<uint16_t> localPort;
    std::optional<std::string> userName;
    std::optional<std::string> userPass;
    std::optional<bool> showConfig;

    CONFIG() = default;
    CONFIG(const CONFIG&) = default;
    CONFIG(CONFIG&&) = default;

    CONFIG& operator=(const CONFIG&) = delete;
    CONFIG& operator=(CONFIG&&) = delete;

    void splice(const CONFIG & rhs) noexcept
    {
        STG::splice(configFile, rhs.configFile);
        STG::splice(server, rhs.server);
        STG::splice(port, rhs.port);
        STG::splice(localAddress, rhs.localAddress);
        STG::splice(localPort, rhs.localPort);
        STG::splice(userName, rhs.userName);
        STG::splice(userPass, rhs.userPass);
        STG::splice(showConfig, rhs.showConfig);
    }

    std::string Serialize() const
    {
    std::string res;
    if (configFile)
        res += "configFile: '" + configFile.value() + "'\n";
    if (server)
        res += "server: '" + server.value() + "'\n";
    if (port)
        res += "port: " + std::to_string(port.value()) + "\n";
    if (localAddress)
        res += "local address: '" + localAddress.value() + "'\n";
    if (localPort)
        res += "local port: " + std::to_string(localPort.value()) + "\n";
    if (userName)
        res += "userName: '" + userName.value() + "'\n";
    if (userPass)
        res += "userPass: '" + userPass.value() + "\n";
    return res;
    }
};

} // namespace SGCONF
