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

namespace STG
{

struct Config
{
    typedef std::map<std::string, std::string> Pairs;
    typedef std::pair<std::string, std::string> Pair;
    enum Type { UNIX, TCP };

    Config(const MODULE_SETTINGS& settings);

    Pairs match;
    Pairs modify;
    Pairs reply;

    bool verbose;

    Type connectionType;
    std::string bindAddress;
    std::string portStr;
    uint16_t port;
    std::string key;
};

} // namespace STG

#endif
