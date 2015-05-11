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

#include <csignal>
#include <cerror>
#include <cstring>

namespace
{

PLUGIN_CREATOR<RADIUS> creator;

}

extern "C" PLUGIN * GetPlugin()
{
    return creator.GetPlugin();
}

RADIUS::RADIUS()
    : m_running(false),
      m_stopped(true),
      m_users(NULL),
      m_store(NULL),
      m_logger(GetPluginLogger(GetStgLogger(), "radius"))
{
}

int RADIUS::ParseSettings()
{
    try {
        m_config = STG::Config(m_settings);
        return 0;
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
            m_error = std::string("'select' is failed: '") + strerror(errno) + "'.";
            m_logger(m_error);
            break;
        }

        if (!m_running)
            break;

        if (res > 0)
            handleEvents(fds);

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
        if (((*pos)->isDone() && !(*pos)->isKeepAlive()) || !(*pos)->isOk()) {
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
    }
}
