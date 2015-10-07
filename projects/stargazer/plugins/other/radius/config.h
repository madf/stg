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

#ifndef __STG_RADIUS_CONFIG_H__
#define __STG_RADIUS_CONFIG_H__

#include "stg/module_settings.h"

#include "stg/os_int.h"

#include <map>
#include <string>

#include <unistd.h> // uid_t, gid_t
#include <sys/stat.h> // mode_t

namespace STG
{

struct Config
{
    typedef std::map<std::string, std::string> Pairs;
    typedef std::pair<std::string, std::string> Pair;
    enum Type { UNIX, TCP };
    enum ReturnCode
    {
        REJECT,   // Reject the request immediately.
        FAIL,     // Module failed.
        OK,       // Module is OK, continue.
        HANDLED,  // The request is handled, no further handling.
        INVALID,  // The request is invalud.
        USERLOCK, // Reject the request, user is locked.
        NOTFOUND, // User not found.
        NOOP,     // Module performed no action.
        UPDATED   // Module sends some updates.
    };

    struct Section
    {
        Section() {}
        Section(const Pairs& ma, const Pairs& mo, const Pairs& re, ReturnCode code)
            : match(ma), modify(mo), reply(re), returnCode(code) {}
        Pairs match;
        Pairs modify;
        Pairs reply;
        ReturnCode returnCode;
    };

    Config() {}
    Config(const MODULE_SETTINGS& settings);

    Section autz;
    Section auth;
    Section postauth;
    Section preacct;
    Section acct;

    bool verbose;

    std::string address;
    std::string bindAddress;
    Type connectionType;
    std::string portStr;
    uint16_t port;
    std::string key;

    uid_t sockUID;
    gid_t sockGID;
    mode_t sockMode;
};

} // namespace STG

#endif
