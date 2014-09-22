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

#ifndef CONFIGPROTO_H
#define CONFIGPROTO_H

#include "parser.h"

#include "stg/module_settings.h"
#include "stg/os_int.h"

#include <string>
#include <deque>

#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>

class SETTINGS;
class ADMINS;
class TARIFFS;
class USERS;
class STORE;
class PLUGIN_LOGGER;

namespace STG
{

class Conn;

}

class CONFIGPROTO {
public:
    CONFIGPROTO(PLUGIN_LOGGER & l);
    ~CONFIGPROTO();

    void            SetPort(uint16_t port) { m_port = port; }
    void            SetBindAddress(const std::string & address) { m_bindAddress = address; }
    void            SetSettings(const SETTINGS * settings) { m_settings = settings; }
    void            SetAdmins(ADMINS * admins) { m_admins = admins; }
    void            SetTariffs(TARIFFS * tariffs) { m_tariffs = tariffs; }
    void            SetUsers(USERS * users) { m_users = users; }
    void            SetStore(STORE * store) { m_store = store; }

    int             Prepare();
    int             Stop();
    const std::string & GetStrError() const { return m_errorStr; }
    void            Run();

private:
    CONFIGPROTO(const CONFIGPROTO & rvalue);
    CONFIGPROTO & operator=(const CONFIGPROTO & rvalue);

    const SETTINGS * m_settings;
    ADMINS *         m_admins;
    TARIFFS *        m_tariffs;
    USERS *          m_users;
    STORE *          m_store;

    uint16_t         m_port;
    std::string      m_bindAddress;
    bool             m_running;
    bool             m_stopped;
    PLUGIN_LOGGER &  m_logger;
    int              m_listenSocket;

    std::string      m_errorStr;

    BASE_PARSER::REGISTRY m_registry;
    std::deque<STG::Conn *> m_conns;

    bool Bind();

    void RegisterParsers();

    int MaxFD() const;
    void BuildFDSet(fd_set & fds) const;
    void CleanupConns();
    void HandleEvents(const fd_set & fds);
    void AcceptConnection();

    //void WriteLogAccessFailed(uint32_t ip);
};

#endif //CONFIGPROTO_H
