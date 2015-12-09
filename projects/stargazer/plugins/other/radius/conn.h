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

#include <boost/scoped_ptr.hpp>

#include <string>

class USER;
class USERS;
class PLUGIN_LOGGER;
class RADIUS;

namespace STG
{

struct Config;

class Conn
{
    public:
        Conn(USERS& users, PLUGIN_LOGGER& logger, RADIUS& plugin, const Config& config, int fd, const std::string& remote);
        ~Conn();

        int sock() const;

        bool read();
        bool tick();

        bool isOk() const;

    private:
        class Impl;
        boost::scoped_ptr<Impl> m_impl;
};

}

#endif
