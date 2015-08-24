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

#ifndef __STG_RADIUS_H__
#define __STG_RADIUS_H__

#include "stg/os_int.h"
#include "stg/auth.h"
#include "stg/module_settings.h"
#include "stg/logger.h"

#include "config.h"
#include "conn.h"

#include <string>
#include <deque>

#include <pthread.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>

extern "C" PLUGIN * GetPlugin();

class STORE;
class USERS;

class RADIUS : public AUTH {
public:
    RADIUS();
    virtual ~RADIUS() {}

    void SetUsers(USERS* u) { m_users = u; }
    void SetStore(STORE* s) { m_store = s; }
    void SetStgSettings(const SETTINGS*) {}
    void SetSettings(const MODULE_SETTINGS& s) { m_settings = s; }
    int ParseSettings();

    int Start();
    int Stop();
    int Reload() { return 0; }
    bool IsRunning() { return m_running; }

    const std::string& GetStrError() const { return m_error; }
    std::string GetVersion() const { return "RADIUS data access plugin v 1.0"; }
    uint16_t GetStartPosition() const { return 30; }
    uint16_t GetStopPosition() const { return 30; }

    int SendMessage(const STG_MSG&, uint32_t) const { return 0; }

private:
    RADIUS(const RADIUS & rvalue);
    RADIUS & operator=(const RADIUS & rvalue);

    static void* run(void*);

    bool reconnect();
    int createUNIX() const;
    int createTCP() const;
    void runImpl();
    int maxFD() const;
    void buildFDSet(fd_set & fds) const;
    void cleanupConns();
    void handleEvents(const fd_set & fds);
    void acceptConnection();
    void acceptUNIX();
    void acceptTCP();

    mutable std::string m_error;
    STG::Config m_config;

    MODULE_SETTINGS m_settings;

    bool m_running;
    bool m_stopped;

    USERS* m_users;
    const STORE* m_store;

    int m_listenSocket;
    std::deque<STG::Conn*> m_conns;

    pthread_t m_thread;

    PLUGIN_LOGGER m_logger;
};

#endif
