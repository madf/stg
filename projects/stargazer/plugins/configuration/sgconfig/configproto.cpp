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

#include "configproto.h"

#include "conn.h"

#include "parser_server_info.h"
#include "parser_admins.h"
#include "parser_tariffs.h"
#include "parser_users.h"
#include "parser_services.h"
#include "parser_message.h"
#include "parser_user_info.h"
#include "parser_auth_by.h"

#include "stg/common.h"
#include "stg/logger.h"

#include <algorithm>
#include <functional>
#include <vector>
#include <csignal>
#include <cstring>
#include <cerrno>
#include <cassert>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

namespace SP = STG::PARSER;

CONFIGPROTO::CONFIGPROTO(PLUGIN_LOGGER & l)
    : m_settings(NULL),
      m_admins(NULL),
      m_tariffs(NULL),
      m_users(NULL),
      m_services(NULL),
      m_corporations(NULL),
      m_store(NULL),
      m_port(0),
      m_bindAddress("0.0.0.0"),
      m_running(false),
      m_stopped(true),
      m_logger(l),
      m_listenSocket(-1)
{
}

CONFIGPROTO::~CONFIGPROTO()
{
    std::deque<STG::Conn *>::iterator it;
    for (it = m_conns.begin(); it != m_conns.end(); ++it)
        delete *it;
}

int CONFIGPROTO::Prepare()
{
    sigset_t sigmask, oldmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGINT);
    sigaddset(&sigmask, SIGTERM);
    sigaddset(&sigmask, SIGUSR1);
    sigaddset(&sigmask, SIGHUP);
    pthread_sigmask(SIG_BLOCK, &sigmask, &oldmask);
    m_listenSocket = socket(PF_INET, SOCK_STREAM, 0);

    if (m_listenSocket < 0)
    {
        m_errorStr = std::string("Cannot create listen socket: '") + strerror(errno) + "'.";
        m_logger(m_errorStr);
        return -1;
    }

    int dummy = 1;

    if (setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, &dummy, 4) != 0)
    {
        m_errorStr = std::string("Failed to set SO_REUSEADDR to the listen socket: '") + strerror(errno) + "'.";
        m_logger(m_errorStr);
        return -1;
    }

    if (!Bind())
        return -1;

    if (listen(m_listenSocket, 64) == -1) // TODO: backlog length
    {
        m_errorStr = std::string("Failed to start listening for connections: '") + strerror(errno) + "'.";
        m_logger(m_errorStr);
        return -1;
    }

    RegisterParsers();

    m_running = true;
    m_stopped = false;
    return 0;
}

int CONFIGPROTO::Stop()
{
    m_running = false;
    for (int i = 0; i < 5 && !m_stopped; ++i)
    {
        struct timespec ts = {0, 200000000};
        nanosleep(&ts, NULL);
    }

    if (!m_stopped)
    {
        m_errorStr = "Cannot stop listenign thread.";
        m_logger(m_errorStr);
        return -1;
    }

    shutdown(m_listenSocket, SHUT_RDWR);
    close(m_listenSocket);
    return 0;
}

void CONFIGPROTO::Run()
{
    while (m_running)
    {
        fd_set fds;

        BuildFDSet(fds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500000;

        int res = select(MaxFD() + 1, &fds, NULL, NULL, &tv);
        if (res < 0)
        {
            m_errorStr = std::string("'select' is failed: '") + strerror(errno) + "'.";
            printfd(__FILE__, "%s\n", m_errorStr.c_str());
            m_logger(m_errorStr);
            break;
        }
        if (!m_running)
            break;
        if (res > 0)
            HandleEvents(fds);

        CleanupConns();
    }
    m_stopped = true;
}

bool CONFIGPROTO::Bind()
{
    const hostent * he = gethostbyname(m_bindAddress.c_str());
    if (he == NULL)
    {
        m_errorStr = "Failed to resolve name '" + m_bindAddress + "': '" + hstrerror(h_errno) + "'.";
        printfd(__FILE__, "%s\n", m_errorStr.c_str());
        m_logger(m_errorStr);
        return false;
    }

    char ** ptr = he->h_addr_list;
    while (*ptr != NULL)
    {
        struct sockaddr_in listenAddr;
        listenAddr.sin_family = PF_INET;
        listenAddr.sin_port = htons(m_port);
        listenAddr.sin_addr.s_addr = *reinterpret_cast<in_addr_t *>(*ptr);

        printfd(__FILE__, "Trying to bind to %s:%d\n", inet_ntostring(listenAddr.sin_addr.s_addr).c_str(), m_port);

        if (bind(m_listenSocket, reinterpret_cast<sockaddr *>(&listenAddr), sizeof(listenAddr)) == 0)
            return true;

        m_errorStr = std::string("Cannot bind listen socket: '") + strerror(errno) + "'.";
        printfd(__FILE__, "%s\n", m_errorStr.c_str());
        m_logger(m_errorStr);

        ++ptr;
    }

    return false;
}

void CONFIGPROTO::RegisterParsers()
{
    assert(m_settings != NULL);
    assert(m_store != NULL);
    assert(m_admins != NULL);
    assert(m_users != NULL);
    assert(m_tariffs != NULL);
    assert(m_services != NULL);
    assert(m_corporations != NULL);

    SP::GET_SERVER_INFO::FACTORY::Register(m_registry, *m_settings, *m_users, *m_tariffs);

    SP::GET_ADMINS::FACTORY::Register(m_registry, *m_admins);
    SP::ADD_ADMIN::FACTORY::Register(m_registry, *m_admins);
    SP::DEL_ADMIN::FACTORY::Register(m_registry, *m_admins);
    SP::CHG_ADMIN::FACTORY::Register(m_registry, *m_admins);

    SP::GET_TARIFFS::FACTORY::Register(m_registry, *m_tariffs);
    SP::ADD_TARIFF::FACTORY::Register(m_registry, *m_tariffs);
    SP::DEL_TARIFF::FACTORY::Register(m_registry, *m_tariffs, *m_users);
    SP::CHG_TARIFF::FACTORY::Register(m_registry, *m_tariffs);

    SP::GET_USERS::FACTORY::Register(m_registry, *m_users);
    SP::GET_USER::FACTORY::Register(m_registry, *m_users);
    SP::ADD_USER::FACTORY::Register(m_registry, *m_users);
    SP::DEL_USER::FACTORY::Register(m_registry, *m_users);
    SP::CHG_USER::FACTORY::Register(m_registry, *m_users, *m_store, *m_tariffs);
    SP::CHECK_USER::FACTORY::Register(m_registry, *m_users);

    SP::GET_SERVICES::FACTORY::Register(m_registry, *m_services);
    SP::GET_SERVICE::FACTORY::Register(m_registry, *m_services);
    SP::ADD_SERVICE::FACTORY::Register(m_registry, *m_services);
    SP::DEL_SERVICE::FACTORY::Register(m_registry, *m_services);
    SP::CHG_SERVICE::FACTORY::Register(m_registry, *m_services);

    SP::SEND_MESSAGE::FACTORY::Register(m_registry, *m_users);

    SP::AUTH_BY::FACTORY::Register(m_registry, *m_users);

    SP::USER_INFO::FACTORY::Register(m_registry, *m_users);
}

int CONFIGPROTO::MaxFD() const
{
    int maxFD = m_listenSocket;
    std::deque<STG::Conn *>::const_iterator it;
    for (it = m_conns.begin(); it != m_conns.end(); ++it)
        if (maxFD < (*it)->Sock())
            maxFD = (*it)->Sock();
    return maxFD;
}

void CONFIGPROTO::BuildFDSet(fd_set & fds) const
{
    FD_ZERO(&fds);
    FD_SET(m_listenSocket, &fds);
    std::deque<STG::Conn *>::const_iterator it;
    for (it = m_conns.begin(); it != m_conns.end(); ++it)
        FD_SET((*it)->Sock(), &fds);
}

void CONFIGPROTO::CleanupConns()
{
    std::deque<STG::Conn *>::iterator pos;
    for (pos = m_conns.begin(); pos != m_conns.end(); ++pos)
        if (((*pos)->IsDone() && !(*pos)->IsKeepAlive()) || !(*pos)->IsOk())
        {
            delete *pos;
            *pos = NULL;
        }

    pos = std::remove(m_conns.begin(), m_conns.end(), static_cast<STG::Conn *>(NULL));
    m_conns.erase(pos, m_conns.end());
}

void CONFIGPROTO::HandleEvents(const fd_set & fds)
{
    if (FD_ISSET(m_listenSocket, &fds))
        AcceptConnection();
    else
    {
        std::deque<STG::Conn *>::iterator it;
        for (it = m_conns.begin(); it != m_conns.end(); ++it)
            if (FD_ISSET((*it)->Sock(), &fds))
                (*it)->Read();
    }
}

void CONFIGPROTO::AcceptConnection()
{
    struct sockaddr_in outerAddr;
    socklen_t outerAddrLen(sizeof(outerAddr));
    int sock = accept(m_listenSocket, reinterpret_cast<sockaddr *>(&outerAddr), &outerAddrLen);

    if (sock < 0)
    {
        m_errorStr = std::string("Failed to accept connection: '") + strerror(errno) + "'.";
        printfd(__FILE__, "%s\n", m_errorStr.c_str());
        m_logger(m_errorStr);
        return;
    }

    assert(m_admins != NULL);

    try
    {
        m_conns.push_back(new STG::Conn(m_registry, *m_admins, sock, outerAddr, m_logger));
        printfd(__FILE__, "New connection from %s:%d. Total connections: %d\n", inet_ntostring(m_conns.back()->IP()).c_str(), m_conns.back()->Port(), m_conns.size());
    }
    catch (const STG::Conn::Error & error)
    {
        // Unlikely.
        m_logger(std::string("Failed to create new client connection: '") + error.what() + "'.");
    }
}
