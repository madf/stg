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

#pragma once

#include "parser.h"

#include "stg/module_settings.h"

#include <string>
#include <deque>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop
#include <cstdint>

#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>

namespace STG
{

struct Settings;
struct Admins;
struct Tariffs;
struct Users;
struct Services;
struct Corporations;
struct Store;
class PluginLogger;

class Conn;

}

class CONFIGPROTO {
public:
    explicit CONFIGPROTO(STG::PluginLogger & l);
    ~CONFIGPROTO();

    void SetPort(uint16_t port) { m_port = port; }
    void SetBindAddress(const std::string & address) { m_bindAddress = address; }
    void SetSettings(const STG::Settings * settings) { m_settings = settings; }
    void SetAdmins(STG::Admins * admins) { m_admins = admins; }
    void SetTariffs(STG::Tariffs * tariffs) { m_tariffs = tariffs; }
    void SetUsers(STG::Users * users) { m_users = users; }
    void SetStore(STG::Store * store) { m_store = store; }
    void SetServices(STG::Services * services) { m_services = services; }
    void SetCorporations(STG::Corporations * corporations) { m_corporations = corporations; }

    int Prepare();
    int Stop();
    const std::string & GetStrError() const { return m_errorStr; }
    void Run(std::stop_token token);

private:
    CONFIGPROTO(const CONFIGPROTO & rvalue);
    CONFIGPROTO & operator=(const CONFIGPROTO & rvalue);

    const STG::Settings * m_settings;
    STG::Admins *         m_admins;
    STG::Tariffs *        m_tariffs;
    STG::Users *          m_users;
    STG::Services *       m_services;
    STG::Corporations *   m_corporations;
    STG::Store *          m_store;

    uint16_t         m_port;
    std::string      m_bindAddress;
    bool             m_stopped;
    STG::PluginLogger &   m_logger;
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
};
