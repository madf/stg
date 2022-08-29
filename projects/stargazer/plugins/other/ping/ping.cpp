#include "ping.h"

#include "stg/user.h"
#include "stg/locker.h"
#include "stg/user_property.h"

#include <algorithm>
#include <cstdio>
#include <cassert>
#include <csignal>
#include <ctime>

using STG::PING;
using STG::PING_SETTINGS;

namespace
{

extern "C" STG::Plugin* GetPlugin()
{
    static PING plugin;
    return &plugin;
}

}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int PING_SETTINGS::ParseSettings(const ModuleSettings & s)
{
ParamValue pv;
std::vector<ParamValue>::const_iterator pvi;

pv.param = "PingDelay";
pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
    errorStr = "Parameter \'PingDelay\' not found.";
    printfd(__FILE__, "Parameter 'PingDelay' not found\n");
    return -1;
    }
if (ParseIntInRange(pvi->value[0], 5, 3600, &pingDelay) != 0)
    {
    errorStr = "Cannot parse parameter \'PingDelay\': " + errorStr;
    printfd(__FILE__, "Canot parse parameter 'PingDelay'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
PING::PING()
    : users(nullptr),
      isRunning(false),
      logger(PluginLogger::get("ping"))
{
}
//-----------------------------------------------------------------------------
int PING::ParseSettings()
{
auto ret = pingSettings.ParseSettings(settings);
if (ret != 0)
    errorStr = pingSettings.GetStrError();
return ret;
}
//-----------------------------------------------------------------------------
int PING::Start()
{
GetUsers();

m_onAddUserConn = users->onAdd([this](auto user){ AddUser(user); });
m_onDelUserConn = users->onDel([this](auto user){ DelUser(user); });

m_pinger.SetDelayTime(pingSettings.GetPingDelay());
m_pinger.Start();

m_thread = std::jthread([this](auto token){ Run(std::move(token)); });

return 0;
}
//-----------------------------------------------------------------------------
int PING::Stop()
{
std::lock_guard lock(m_mutex);

if (!m_thread.joinable())
    return 0;

m_pinger.Stop();
m_thread.request_stop();
//5 seconds to thread stops itself
struct timespec ts = {0, 200000000};
for (int i = 0; i < 25; i++)
    {
    if (!isRunning)
        break;

    nanosleep(&ts, nullptr);
    }

m_onAddUserConn.disconnect();
m_onDelUserConn.disconnect();

m_conns.clear();

if (isRunning)
    m_thread.detach();
else
    m_thread.join();

return 0;
}
//-----------------------------------------------------------------------------
bool PING::IsRunning()
{
return isRunning;
}
//-----------------------------------------------------------------------------
void PING::Run(std::stop_token token)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, nullptr);

isRunning = true;

long delay = (10000000 * pingSettings.GetPingDelay()) / 3 + 50000000;

while (!token.stop_requested())
    {
    auto iter = usersList.begin();
        {
        std::lock_guard lock(m_mutex);
        while (iter != usersList.end())
            {
            if ((*iter)->GetProperties().ips.ConstData().onlyOneIP())
                {
                uint32_t ip = (*iter)->GetProperties().ips.ConstData()[0].ip;
                time_t t;
                if (m_pinger.GetIPTime(ip, t) == 0)
                    {
                    if (t != 0)
                        (*iter)->UpdatePingTime(t);
                    }
                }
            else
                {
                uint32_t ip = (*iter)->GetCurrIP();
                if (ip != 0)
                    {
                    time_t t;
                    if (m_pinger.GetIPTime(ip, t) == 0)
                        {
                        if (t != 0)
                            (*iter)->UpdatePingTime(t);
                        }
                    }
                }
            ++iter;
            }
        }
    struct timespec ts = {delay / 1000000000, delay % 1000000000};
    for (int i = 0; i < 100; i++)
        {
        if (!token.stop_requested())
            {
            nanosleep(&ts, nullptr);
            }
        }
    }

isRunning = false;
}
//-----------------------------------------------------------------------------
void PING::SetUserNotifiers(UserPtr u)
{
    m_conns.emplace_back(
        u->GetID(),
        u->afterCurrIPChange([this](auto oldVal, auto newVal){ updateCurrIP(oldVal, newVal); }),
        u->GetProperties().ips.afterChange([this](const auto& oldVal, const auto& newVal){ updateIPs(oldVal, newVal); })
    );
}
//-----------------------------------------------------------------------------
void PING::UnSetUserNotifiers(UserPtr u)
{
    m_conns.erase(std::remove_if(m_conns.begin(), m_conns.end(),
                                 [u](const auto& c){ return std::get<0>(c) == u->GetID(); }),
                  m_conns.end());
}
//-----------------------------------------------------------------------------
void PING::GetUsers()
{
std::lock_guard lock(m_mutex);

UserPtr u;
int h = users->OpenSearch();
assert(h && "USERS::OpenSearch is always correct");

while (users->SearchNext(h, &u) == 0)
    {
    usersList.push_back(u);
    SetUserNotifiers(u);
    if (u->GetProperties().ips.ConstData().onlyOneIP())
        {
        m_pinger.AddIP(u->GetProperties().ips.ConstData()[0].ip);
        }
    else
        {
        uint32_t ip = u->GetCurrIP();
        if (ip != 0)
            m_pinger.AddIP(ip);
        }
    }

users->CloseSearch(h);
}
//-----------------------------------------------------------------------------
void PING::AddUser(UserPtr u)
{
std::lock_guard lock(m_mutex);

SetUserNotifiers(u);
usersList.push_back(u);
}
//-----------------------------------------------------------------------------
void PING::DelUser(UserPtr u)
{
std::lock_guard lock(m_mutex);

UnSetUserNotifiers(u);

std::list<UserPtr>::iterator users_iter;
users_iter = usersList.begin();

while (users_iter != usersList.end())
    {
    if (u == *users_iter)
        {
        usersList.erase(users_iter);
        break;
        }
    ++users_iter;
    }
}
//-----------------------------------------------------------------------------
void PING::updateCurrIP(uint32_t oldVal, uint32_t newVal)
{
    m_pinger.DelIP(oldVal);
    if (newVal != 0)
        m_pinger.AddIP(newVal);
}
//-----------------------------------------------------------------------------
void PING::updateIPs(const UserIPs& oldVal, const UserIPs& newVal)
{
    if (oldVal.onlyOneIP())
        m_pinger.DelIP(oldVal[0].ip);

    if (newVal.onlyOneIP())
        m_pinger.AddIP(newVal[0].ip);
}
