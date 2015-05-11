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

#ifndef __STG_SGCONFIG_CONN_H__
#define __STG_SGCONFIG_CONN_H__

#include "stg/os_int.h"

#include <stdexcept>
#include <string>

class USERS;

namespace STG
{

class Conn
{
    public:
        struct Error : public std::runtime_error
        {
            Error(const std::string& message) : runtime_error(message.c_str()) {}
        };

        Conn(USERS& users, PLUGIN_LOGGER& logger, const Config& config);
        ~Conn();

        int sock() const { return m_sock; }

        bool read();

    private:
        USERS& m_users;
        PLUGIN_LOGGER& m_logger;
        const Config& m_config;
};

}

#endif
