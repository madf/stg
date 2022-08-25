#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <cstring>
#include <cerrno>
#include <ctime>
#include <csignal>
#include <cassert>

#include <vector>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "stg/common.h"

#include "smux.h"
#include "utils.h"

namespace
{

bool SPrefixLess(const Sensors::value_type & a,
                 const Sensors::value_type & b)
{
return a.first.PrefixLess(b.first);
}

}

extern "C" STG::Plugin* GetPlugin()
{
    static SMUX plugin;
    return &plugin;
}

SMUX_SETTINGS::SMUX_SETTINGS()
    : ip(0),
      port(0)
{}

int SMUX_SETTINGS::ParseSettings(const STG::ModuleSettings & s)
{
STG::ParamValue pv;
int p;

pv.param = "Port";
auto pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
    errorStr = "Parameter \'Port\' not found.";
    printfd(__FILE__, "Parameter 'Port' not found\n");
    return -1;
    }
if (ParseIntInRange(pvi->value[0], 2, 65535, &p) != 0)
    {
    errorStr = "Cannot parse parameter \'Port\': " + errorStr;
    printfd(__FILE__, "Cannot parse parameter 'Port'\n");
    return -1;
    }
port = static_cast<uint16_t>(p);

pv.param = "Password";
pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
    errorStr = "Parameter \'Password\' not found.";
    printfd(__FILE__, "Parameter 'Password' not found\n");
    password = "";
    }
else
    {
    password = pvi->value[0];
    }

pv.param = "Server";
pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
    errorStr = "Parameter \'Server\' not found.";
    printfd(__FILE__, "Parameter 'Server' not found\n");
    return -1;
    }
ip = inet_strington(pvi->value[0]);

return 0;
}

SMUX::SMUX()
    : users(nullptr),
      tariffs(nullptr),
      admins(nullptr),
      services(nullptr),
      corporations(nullptr),
      traffcounter(nullptr),
      stopped(true),
      needReconnect(false),
      lastReconnectTry(0),
      reconnectTimeout(1),
      sock(-1),
      logger(STG::PluginLogger::get("smux"))
{
smuxHandlers[SMUX_PDUs_PR_close] = &SMUX::CloseHandler;
smuxHandlers[SMUX_PDUs_PR_registerResponse] = &SMUX::RegisterResponseHandler;
smuxHandlers[SMUX_PDUs_PR_pdus] = &SMUX::PDUsRequestHandler;
smuxHandlers[SMUX_PDUs_PR_commitOrRollback] = &SMUX::CommitOrRollbackHandler;

pdusHandlers[PDUs_PR_get_request] = &SMUX::GetRequestHandler;
pdusHandlers[PDUs_PR_get_next_request] = &SMUX::GetNextRequestHandler;
pdusHandlers[PDUs_PR_set_request] = &SMUX::SetRequestHandler;
}

SMUX::~SMUX()
{
    for (auto& kv : sensors)
        delete kv.second;
    for (auto& kv : tables)
        delete kv.second;
    printfd(__FILE__, "SMUX::~SMUX()\n");
}

int SMUX::ParseSettings()
{
return smuxSettings.ParseSettings(settings);
}

int SMUX::Start()
{
assert(users != nullptr && "users must not be NULL");
assert(tariffs != nullptr && "tariffs must not be NULL");
assert(admins != nullptr && "admins must not be NULL");
assert(services != nullptr && "services must not be NULL");
assert(corporations != nullptr && "corporations must not be NULL");
assert(traffcounter != nullptr && "traffcounter must not be NULL");

if (PrepareNet())
    needReconnect = true;

// Users
sensors[OID(".1.3.6.1.4.1.38313.1.1.1")] = new TotalUsersSensor(*users);
sensors[OID(".1.3.6.1.4.1.38313.1.1.2")] = new ConnectedUsersSensor(*users);
sensors[OID(".1.3.6.1.4.1.38313.1.1.3")] = new AuthorizedUsersSensor(*users);
sensors[OID(".1.3.6.1.4.1.38313.1.1.4")] = new AlwaysOnlineUsersSensor(*users);
sensors[OID(".1.3.6.1.4.1.38313.1.1.5")] = new NoCashUsersSensor(*users);
sensors[OID(".1.3.6.1.4.1.38313.1.1.6")] = new DisabledDetailStatsUsersSensor(*users);
sensors[OID(".1.3.6.1.4.1.38313.1.1.7")] = new DisabledUsersSensor(*users);
sensors[OID(".1.3.6.1.4.1.38313.1.1.8")] = new PassiveUsersSensor(*users);
sensors[OID(".1.3.6.1.4.1.38313.1.1.9")] = new CreditUsersSensor(*users);
sensors[OID(".1.3.6.1.4.1.38313.1.1.10")] = new FreeMbUsersSensor(*users);
sensors[OID(".1.3.6.1.4.1.38313.1.1.11")] = new TariffChangeUsersSensor(*users);
sensors[OID(".1.3.6.1.4.1.38313.1.1.12")] = new ActiveUsersSensor(*users);
// Tariffs
sensors[OID(".1.3.6.1.4.1.38313.1.2.1")] = new TotalTariffsSensor(*tariffs);
// Admins
sensors[OID(".1.3.6.1.4.1.38313.1.3.1")] = new TotalAdminsSensor(*admins);
// Services
sensors[OID(".1.3.6.1.4.1.38313.1.4.1")] = new TotalServicesSensor(*services);
// Corporations
sensors[OID(".1.3.6.1.4.1.38313.1.5.1")] = new TotalCorporationsSensor(*corporations);
// Traffcounter
sensors[OID(".1.3.6.1.4.1.38313.1.6.1")] = new TotalRulesSensor(*traffcounter);

// Table data
tables[".1.3.6.1.4.1.38313.1.2.2"] = new TariffUsersTable(".1.3.6.1.4.1.38313.1.2.2", *tariffs, *users);

UpdateTables();
SetNotifiers();

#ifdef SMUX_DEBUG
auto it = sensors.begin();
while (it != sensors.end())
    {
    printfd(__FILE__, "%s = %s\n",
            it->first.ToString().c_str(),
            it->second->ToString().c_str());
    ++it;
    }
#endif

if (!m_thread.joinable())
    m_thread = std::jthread([this](auto token){ Run(std::move(token)); });

return 0;
}

int SMUX::Stop()
{
printfd(__FILE__, "SMUX::Stop() - Before\n");
m_thread.request_stop();

if (!stopped)
    {
    //5 seconds to thread stops itself
    for (int i = 0; i < 25 && !stopped; i++)
        {
        struct timespec ts = {0, 200000000};
        nanosleep(&ts, nullptr);
        }
    }

if (!stopped)
    m_thread.detach();
else
    m_thread.join();

ResetNotifiers();

for (auto& kv : sensors)
    delete kv.second;
for (auto& kv : tables)
    delete kv.second;

tables.erase(tables.begin(), tables.end());
sensors.erase(sensors.begin(), sensors.end());

close(sock);

if (!stopped)
    {
    return -1;
    }

printfd(__FILE__, "SMUX::Stop() - After\n");
return 0;
}

int SMUX::Reload(const STG::ModuleSettings & /*ms*/)
{
if (Stop() != 0)
    return -1;
if (Start() != 0)
    return -1;
if (!needReconnect)
    {
    printfd(__FILE__, "SMUX reconnected succesfully.\n");
    logger("Reconnected successfully.");
    }
return 0;
}

void SMUX::Run(std::stop_token token)
{
stopped = true;
if (!SendOpenPDU(sock))
    needReconnect = true;
if (!SendRReqPDU(sock))
    needReconnect = true;
stopped = false;

while (!token.stop_requested())
    {
    if (WaitPackets(sock) && !needReconnect)
        {
        auto* pdus = RecvSMUXPDUs(sock);
        if (pdus != nullptr)
            {
            DispatchPDUs(pdus);
            ASN_STRUCT_FREE(asn_DEF_SMUX_PDUs, pdus);
            }
        else if (!token.stop_requested())
            Reconnect();
        }
    else if (!token.stop_requested() && needReconnect)
        Reconnect();
    if (token.stop_requested())
        break;
    }
SendClosePDU(sock);
stopped = true;
}

bool SMUX::PrepareNet()
{
sock = socket(AF_INET, SOCK_STREAM, 0);

if (sock < 0)
    {
    errorStr = "Cannot create socket.";
    logger("Cannot create a socket: %s", strerror(errno));
    printfd(__FILE__, "Cannot create socket\n");
    return true;
    }

struct sockaddr_in addr;

addr.sin_family = AF_INET;
addr.sin_port = htons(smuxSettings.GetPort());
addr.sin_addr.s_addr = smuxSettings.GetIP();

if (connect(sock, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) != 0)
    {
    errorStr = "Cannot connect.";
    logger("Cannot connect the socket: %s", strerror(errno));
    printfd(__FILE__, "Cannot connect. Message: '%s'\n", strerror(errno));
    return true;
    }

return false;
}

bool SMUX::Reconnect()
{
if (needReconnect && difftime(time(nullptr), lastReconnectTry) < reconnectTimeout)
    return true;

time(&lastReconnectTry);
SendClosePDU(sock);
close(sock);
if (!PrepareNet())
    if (SendOpenPDU(sock))
        if (SendRReqPDU(sock))
            {
            reconnectTimeout = 1;
            needReconnect = false;
            logger("Connected successfully");
            printfd(__FILE__, "Connected successfully\n");
            return false;
            }

if (needReconnect)
    if (reconnectTimeout < 60)
        reconnectTimeout *= 2;

needReconnect = true;
return true;
}

bool SMUX::DispatchPDUs(const SMUX_PDUs_t * pdus)
{
auto it = smuxHandlers.find(pdus->present);
if (it != smuxHandlers.end())
    return (this->*(it->second))(pdus);
#ifdef SMUX_DEBUG
else
    {
    switch (pdus->present)
        {
        case SMUX_PDUs_PR_NOTHING:
            printfd(__FILE__, "PDUs: nothing\n");
            break;
        case SMUX_PDUs_PR_open:
            printfd(__FILE__, "PDUs: open\n");
            break;
        case SMUX_PDUs_PR_registerRequest:
            printfd(__FILE__, "PDUs: registerRequest\n");
            break;
        default:
            printfd(__FILE__, "PDUs: undefined\n");
        }
    asn_fprint(stderr, &asn_DEF_SMUX_PDUs, pdus);
    }
#endif
return false;
}

bool SMUX::UpdateTables()
{
Sensors newSensors;
bool done = true;
auto it = tables.begin();
while (it != tables.end())
    {
    try
        {
        it->second->UpdateSensors(newSensors);
        }
    catch (const std::runtime_error & ex)
        {
        printfd(__FILE__,
                "SMUX::UpdateTables - failed to update table '%s': '%s'\n",
                it->first.c_str(), ex.what());
        done = false;
        break;
        }
    ++it;
    }
if (!done)
    {
    auto sit = newSensors.begin();
    while (sit != newSensors.end())
        {
        delete sit->second;
        ++sit;
        }
    return false;
    }

it = tables.begin();
while (it != tables.end())
    {
    auto res = std::equal_range(sensors.begin(),
                                sensors.end(),
                                std::pair<OID, Sensor *>(OID(it->first), nullptr),
                                SPrefixLess);
    auto sit = res.first;
    while (sit != res.second)
        {
        delete sit->second;
        ++sit;
        }
    sensors.erase(res.first, res.second);
    ++it;
    }

sensors.insert(newSensors.begin(), newSensors.end());

return true;
}

void SMUX::SetNotifier(UserPtr userPtr)
{
notifiers.emplace_back(*this, userPtr);
userPtr->GetProperties().tariffName.AddAfterNotifier(&notifiers.back());
}

void SMUX::UnsetNotifier(UserPtr userPtr)
{
auto it = notifiers.begin();
while (it != notifiers.end())
    {
    if (it->GetUserPtr() == userPtr)
        {
        userPtr->GetProperties().tariffName.DelAfterNotifier(&(*it));
        notifiers.erase(it);
        break;
        }
    ++it;
    }
}

void SMUX::SetNotifiers()
{
int h = users->OpenSearch();
assert(h && "USERS::OpenSearch is always correct");

UserPtr u;
while (users->SearchNext(h, &u) == 0)
    SetNotifier(u);

users->CloseSearch(h);

m_onAddUserConn = users->onAdd([this](auto user){
    SetNotifier(user);
    UpdateTables();
});
m_onDelUserConn = users->onDel([this](auto user){
    UnsetNotifier(user);
    UpdateTables();
});

auto updateTables = [this](const STG::TariffData&){ UpdateTables(); };
m_onAddTariffConn = tariffs->onAdd(updateTables);
m_onDelTariffConn = tariffs->onDel(updateTables);
}

void SMUX::ResetNotifiers()
{
m_onAddTariffConn.disconnect();
m_onDelTariffConn.disconnect();

m_onAddUserConn.disconnect();
m_onDelUserConn.disconnect();

auto it = notifiers.begin();
while (it != notifiers.end())
    {
    it->GetUserPtr()->GetProperties().tariffName.DelAfterNotifier(&(*it));
    ++it;
    }
notifiers.clear();
}
