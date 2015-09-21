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

#include "stg_client.h"

#include "conn.h"
#include "radlog.h"

#include "stg/locker.h"
#include "stg/common.h"

#include <map>
#include <utility>

using STG::RLM::Client;
using STG::RLM::Conn;
using STG::RLM::RESULT;

namespace {

Client* stgClient = NULL;

}

class Client::Impl
{
    public:
        explicit Impl(const std::string& address);
        ~Impl();

        bool stop() { return m_conn ? m_conn->stop() : true; }

        RESULT request(REQUEST_TYPE type, const std::string& userName, const std::string& password, const PAIRS& pairs);

    private:
        std::string m_address;
        boost::scoped_ptr<Conn> m_conn;

        pthread_mutex_t m_mutex;
        pthread_cond_t m_cond;
        bool m_done;
        RESULT m_result;
        bool m_status;

        static bool callback(void* data, const RESULT& result, bool status)
        {
            Impl& impl = *static_cast<Impl*>(data);
            STG_LOCKER lock(impl.m_mutex);
            impl.m_result = result;
            impl.m_status = status;
            impl.m_done = true;
            pthread_cond_signal(&impl.m_cond);
            return true;
        }
};

Client::Impl::Impl(const std::string& address)
    : m_address(address)
{
    try
    {
        m_conn.reset(new Conn(m_address, &Impl::callback, this));
    }
    catch (const std::runtime_error& ex)
    {
        RadLog("Connection error: %s.", ex.what());
    }
    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_cond, NULL);
    m_done = false;
}

Client::Impl::~Impl()
{
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_mutex);
}

RESULT Client::Impl::request(REQUEST_TYPE type, const std::string& userName, const std::string& password, const PAIRS& pairs)
{
    STG_LOCKER lock(m_mutex);
    if (!m_conn || !m_conn->connected())
        m_conn.reset(new Conn(m_address, &Impl::callback, this));
    if (!m_conn->connected())
        throw Conn::Error("Failed to create connection to '" + m_address + "'.");

    m_done = false;
    m_conn->request(type, userName, password, pairs);
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;
    int res = 0;
    while (!m_done && res == 0)
        res = pthread_cond_timedwait(&m_cond, &m_mutex, &ts);
    if (res != 0 || !m_status)
        throw Conn::Error("Request failed.");
    return m_result;
}

Client::Client(const std::string& address)
    : m_impl(new Impl(address))
{
}

Client::~Client()
{
}

bool Client::stop()
{
    return m_impl->stop();
}

RESULT Client::request(REQUEST_TYPE type, const std::string& userName, const std::string& password, const PAIRS& pairs)
{
    return m_impl->request(type, userName, password, pairs);
}

Client* Client::get()
{
    return stgClient;
}

bool Client::configure(const std::string& address)
{
    if ( stgClient != NULL )
        return stgClient->configure(address);
    try {
        stgClient = new Client(address);
        return true;
    } catch (const std::exception& ex) {
        RadLog("Client configuration error: %s.", ex.what());
    }
    return false;
}
