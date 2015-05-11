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

#include "conn.h"

#include "config.h"

#include "stg/users.h"
#include "stg/user.h"
#include "stg/logger.h"
#include "stg/common.h"

#include <cstring>
#include <cerrno>

using STG::Conn;

Conn::Conn(USERS& users, PLUGIN_LOGGER & logger, const Config& config)
    : m_users(users),
      m_logger(logger),
      m_config(config)
{
}

Conn::~Conn()
{
}

bool Conn::read()
{
    ssize_t res = read(m_sock, m_buffer, m_bufferSize);
    if (res < 0)
    {
        m_state = ERROR;
        Log(__FILE__, "Failed to read data from " + inet_ntostring(IP()) + ":" + x2str(port()) + ". Reason: '" + strerror(errno) + "'");
        return false;
    }
    if (res == 0 && m_state != DATA) // EOF is ok for data.
    {
        m_state = ERROR;
        Log(__FILE__, "Failed to read data from " + inet_ntostring(IP()) + ":" + x2str(port()) + ". Unexpected EOF.");
        return false;
    }
    m_bufferSize -= res;
    return HandleBuffer(res);
}
