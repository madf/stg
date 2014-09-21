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


#include "conn.h"

#include "stg/common.h"
#include "stg/logger.h"

#include <algorithm>
#include <functional>
#include <csignal>
#include <cstring>
#include <cerrno>
#include <cassert>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace
{

struct IsFinished : public std::unary_function<STG::Conn *, bool>
{
    result_type operator()(const argument_type & arg)
    {
        return (arg->IsDone() && !arg->IsKeepAlive()) || !arg->IsOk();
    }
};

struct RemoveConn : public std::unary_function<STG::Conn *, void>
{
    result_type operator()(const argument_type & arg)
    {
        delete arg;
    }
};

}

CONFIGPROTO::CONFIGPROTO(PLUGIN_LOGGER & l)
    : m_settings(NULL),
      m_admins(NULL),
      m_tariffs(NULL),
      m_users(NULL),
      m_port(0),
      m_running(false),
      m_stopped(true),
      m_logger(l),
      m_listenSocket(-1)
{
    std::for_each(m_conns.begin(), m_conns.end(), RemoveConn());
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

    struct sockaddr_in listenAddr;
    listenAddr.sin_family = PF_INET;
    listenAddr.sin_port = htons(m_port);
    listenAddr.sin_addr.s_addr = inet_addr("0.0.0.0"); // TODO: arbitrary address

    int dummy = 1;

    if (setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, &dummy, 4) != 0)
    {
        m_errorStr = std::string("Failed to set SO_REUSEADDR to the listen socket: '") + strerror(errno) + "'.";
        m_logger(m_errorStr);
        return -1;
    }

    if (bind(m_listenSocket, reinterpret_cast<sockaddr *>(&listenAddr), sizeof(listenAddr)) == -1)
    {
        m_errorStr = std::string("Cannot bind listen socket: '") + strerror(errno) + "'.";
        m_logger(m_errorStr);
        return -1;
    }

    if (listen(m_listenSocket, 64) == -1) // TODO: backlog length
    {
        m_errorStr = std::string("Failed to start listening for connections: '") + strerror(errno) + "'.";
        m_logger(m_errorStr);
        return -1;
    }

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

int CONFIGPROTO::MaxFD() const
{
    int maxFD = m_listenSocket;
    for (size_t i = 0; i < m_conns.size(); ++i)
        if (maxFD < m_conns[i]->Sock())
            maxFD = m_conns[i]->Sock();
    return maxFD;
}

void CONFIGPROTO::BuildFDSet(fd_set & fds) const
{
    for (size_t i = 0; i < m_conns.size(); ++i)
        FD_SET(m_conns[i]->Sock(), &fds);
}

void CONFIGPROTO::CleanupConns()
{
    std::vector<STG::Conn *>::iterator pos;
    pos = std::remove_if(m_conns.begin(), m_conns.end(), IsFinished());
    if (pos == m_conns.end())
        return;
    std::for_each(pos, m_conns.end(), RemoveConn());
    m_conns.erase(pos, m_conns.end());
}

void CONFIGPROTO::HandleEvents(const fd_set & fds)
{
    if (FD_ISSET(m_listenSocket, &fds))
        AcceptConnection();
    else
    {
        for (size_t i = 0; i < m_conns.size(); ++i)
            if (FD_ISSET(m_conns[i]->Sock(), &fds))
                m_conns[i]->Read();
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
        printfd(__FILE__, "%s", m_errorStr.c_str());
        m_logger(m_errorStr);
        return;
    }

    assert(m_settings != NULL);
    assert(m_admins != NULL);
    assert(m_users != NULL);
    assert(m_tariffs != NULL);

    m_conns.push_back(new STG::Conn(*m_settings, *m_admins, *m_users, *m_tariffs, sock, outerAddr));

    printfd(__FILE__, "New connection from %s:%d\n", inet_ntostring(m_conns.back()->IP()).c_str(), m_conns.back()->Port());
}
/*
void CONFIGPROTO::WriteLogAccessFailed(uint32_t ip)
{
    m_logger("Admin's connection failed. IP %s", inet_ntostring(ip).c_str());
}
*/
