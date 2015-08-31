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

#include "radius.h"

#include "stg/store.h"
#include "stg/users.h"
#include "stg/plugin_creator.h"
#include "stg/common.h"

#include <algorithm>
#include <stdexcept>
#include <csignal>
#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> // UNIX
#include <netinet/in.h> // IP
#include <netinet/tcp.h> // TCP
#include <netdb.h>

using STG::Config;
using STG::Conn;

namespace
{

PLUGIN_CREATOR<RADIUS> creator;

}

extern "C" PLUGIN * GetPlugin()
{
    return creator.GetPlugin();
}

RADIUS::RADIUS()
    : m_config(),
      m_running(false),
      m_stopped(true),
      m_users(NULL),
      m_store(NULL),
      m_listenSocket(0),
      m_logger(GetPluginLogger(GetStgLogger(), "radius"))
{
}

int RADIUS::ParseSettings()
{
    try {
        m_config = STG::Config(m_settings);
        return reconnect() ? 0 : -1;
    } catch (const std::runtime_error& ex) {
        m_logger("Failed to parse settings. %s", ex.what());
        return -1;
    }
}

int RADIUS::Start()
{
    if (m_running)
        return 0;

    int res = pthread_create(&m_thread, NULL, run, this);
    if (res == 0)
        return 0;

    m_error = strerror(res);
    m_logger("Failed to create thread: '" + m_error + "'.");
    return -1;
}

int RADIUS::Stop()
{
    if (m_stopped)
        return 0;

    m_running = false;

    for (size_t i = 0; i < 25 && !m_stopped; i++) {
        struct timespec ts = {0, 200000000};
        nanosleep(&ts, NULL);
    }

    if (m_stopped) {
        pthread_join(m_thread, NULL);
        return 0;
    }

    if (m_config.connectionType == Config::UNIX)
        unlink(m_config.bindAddress.c_str());

    m_error = "Failed to stop thread.";
    m_logger(m_error);
    return -1;
}
//-----------------------------------------------------------------------------
void* RADIUS::run(void* d)
{
    sigset_t signalSet;
    sigfillset(&signalSet);
    pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

    static_cast<RADIUS *>(d)->runImpl();

    return NULL;
}

bool RADIUS::reconnect()
{
    if (!m_conns.empty())
    {
        std::deque<STG::Conn *>::const_iterator it;
        for (it = m_conns.begin(); it != m_conns.end(); ++it)
            delete(*it);
        m_conns.clear();
    }
    if (m_listenSocket != 0)
    {
        shutdown(m_listenSocket, SHUT_RDWR);
        close(m_listenSocket);
    }
    if (m_config.connectionType == Config::UNIX)
        m_listenSocket = createUNIX();
    else
        m_listenSocket = createTCP();
    if (m_listenSocket == 0)
        return false;
    if (listen(m_listenSocket, 100) == -1)
    {
        m_error = std::string("Error starting to listen socket: ") + strerror(errno);
        m_logger(m_error);
        return false;
    }
    return true;
}

int RADIUS::createUNIX() const
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1)
    {
        m_error = std::string("Error creating UNIX socket: ") + strerror(errno);
        m_logger(m_error);
        return 0;
    }
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, m_config.bindAddress.c_str(), m_config.bindAddress.length());
    unlink(m_config.bindAddress.c_str());
    if (bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1)
    {
        shutdown(fd, SHUT_RDWR);
        close(fd);
        m_error = std::string("Error binding UNIX socket: ") + strerror(errno);
        m_logger(m_error);
        return 0;
    }
    return fd;
}

int RADIUS::createTCP() const
{
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));

    hints.ai_family = AF_INET;       /* Allow IPv4 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = AI_PASSIVE;     /* For wildcard IP address */
    hints.ai_protocol = 0;           /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    addrinfo* ais = NULL;
    int res = getaddrinfo(m_config.bindAddress.c_str(), m_config.portStr.c_str(), &hints, &ais);
    if (res != 0)
    {
        m_error = "Error resolving address '" + m_config.bindAddress + "': " + gai_strerror(res);
        m_logger(m_error);
        return 0;
    }

    for (addrinfo* ai = ais; ai != NULL; ai = ai->ai_next)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1)
        {
            m_error = std::string("Error creating TCP socket: ") + strerror(errno);
            m_logger(m_error);
            freeaddrinfo(ais);
            return 0;
        }
        if (bind(fd, ai->ai_addr, ai->ai_addrlen) == -1)
        {
            shutdown(fd, SHUT_RDWR);
            close(fd);
            m_error = std::string("Error binding TCP socket: ") + strerror(errno);
            m_logger(m_error);
            continue;
        }
        freeaddrinfo(ais);
        return fd;
    }

    m_error = "Failed to resolve '" + m_config.bindAddress;
    m_logger(m_error);

    freeaddrinfo(ais);
    return 0;
}

void RADIUS::runImpl()
{
    m_running = true;

    while (m_running) {
        fd_set fds;

        buildFDSet(fds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500000;

        int res = select(maxFD() + 1, &fds, NULL, NULL, &tv);
        if (res < 0)
        {
            if (errno == EINTR)
                continue;
            m_error = std::string("'select' is failed: '") + strerror(errno) + "'.";
            m_logger(m_error);
            break;
        }

        if (!m_running)
            break;

        if (res > 0)
            handleEvents(fds);
        else
        {
            for (std::deque<Conn*>::iterator it = m_conns.begin(); it != m_conns.end(); ++it)
                (*it)->tick();
        }

        cleanupConns();
    }

    m_stopped = true;
}

int RADIUS::maxFD() const
{
    int maxFD = m_listenSocket;
    std::deque<STG::Conn *>::const_iterator it;
    for (it = m_conns.begin(); it != m_conns.end(); ++it)
        if (maxFD < (*it)->sock())
            maxFD = (*it)->sock();
    return maxFD;
}

void RADIUS::buildFDSet(fd_set & fds) const
{
    FD_ZERO(&fds);
    FD_SET(m_listenSocket, &fds);
    std::deque<STG::Conn *>::const_iterator it;
    for (it = m_conns.begin(); it != m_conns.end(); ++it)
        FD_SET((*it)->sock(), &fds);
}

void RADIUS::cleanupConns()
{
    std::deque<STG::Conn *>::iterator pos;
    for (pos = m_conns.begin(); pos != m_conns.end(); ++pos)
        if (!(*pos)->isOk()) {
            delete *pos;
            *pos = NULL;
        }

    pos = std::remove(m_conns.begin(), m_conns.end(), static_cast<STG::Conn *>(NULL));
    m_conns.erase(pos, m_conns.end());
}

void RADIUS::handleEvents(const fd_set & fds)
{
    if (FD_ISSET(m_listenSocket, &fds))
        acceptConnection();
    else
    {
        std::deque<STG::Conn *>::iterator it;
        for (it = m_conns.begin(); it != m_conns.end(); ++it)
            if (FD_ISSET((*it)->sock(), &fds))
                (*it)->read();
            else
                (*it)->tick();
    }
}

void RADIUS::acceptConnection()
{
    if (m_config.connectionType == Config::UNIX)
        acceptUNIX();
    else
        acceptTCP();
}

void RADIUS::acceptUNIX()
{
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t size = sizeof(addr);
    int res = accept(m_listenSocket, reinterpret_cast<sockaddr*>(&addr), &size);
    if (res == -1)
    {
        m_error = std::string("Failed to accept UNIX connection: ") + strerror(errno);
        m_logger(m_error);
        return;
    }
    printfd(__FILE__, "New UNIX connection: '%s'\n", addr.sun_path);
    m_conns.push_back(new Conn(*m_users, m_logger, m_config, res, addr.sun_path));
}

void RADIUS::acceptTCP()
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t size = sizeof(addr);
    int res = accept(m_listenSocket, reinterpret_cast<sockaddr*>(&addr), &size);
    if (res == -1)
    {
        m_error = std::string("Failed to accept TCP connection: ") + strerror(errno);
        m_logger(m_error);
        return;
    }
    std::string remote = inet_ntostring(addr.sin_addr.s_addr) + ":" + x2str(ntohs(addr.sin_port));
    printfd(__FILE__, "New TCP connection: '%s'\n", remote.c_str());
    m_conns.push_back(new Conn(*m_users, m_logger, m_config, res, remote));
}
