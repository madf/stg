#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <cstring>
#include <cerrno>
#include <ctime>
#include <csignal>

#include <vector>
#include <algorithm>

#include "stg/common.h"
#include "stg/plugin_creator.h"

#include "smux.h"
#include "utils.h"

PLUGIN_CREATOR<SMUX> sac;

PLUGIN * GetPlugin()
{
return sac.GetPlugin();
}

SMUX_SETTINGS::SMUX_SETTINGS()
    : ip(0),
      port(0)
{}

int SMUX_SETTINGS::ParseSettings(const MODULE_SETTINGS & s)
{
PARAM_VALUE pv;
std::vector<PARAM_VALUE>::const_iterator pvi;
int p;
///////////////////////////
pv.param = "Port";
pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end())
    {
    errorStr = "Parameter \'Port\' not found.";
    printfd(__FILE__, "Parameter 'Port' not found\n");
    return -1;
    }
if (ParseIntInRange(pvi->value[0], 2, 65535, &p))
    {
    errorStr = "Cannot parse parameter \'Port\': " + errorStr;
    printfd(__FILE__, "Cannot parse parameter 'Port'\n");
    return -1;
    }
port = p;

pv.param = "Password";
pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end())
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
if (pvi == s.moduleParams.end())
    {
    errorStr = "Parameter \'Server\' not found.";
    printfd(__FILE__, "Parameter 'Server' not found\n");
    return -1;
    }
ip = inet_strington(pvi->value[0]);

return 0;
}

SMUX::SMUX()
    : PLUGIN(),
      users(NULL),
      tariffs(NULL),
      running(false),
      stopped(true),
      sock(-1)
{
pthread_mutex_init(&mutex, NULL);

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
Sensors::iterator it;
for (it = sensors.begin(); it != sensors.end(); ++it)
    delete it->second;
printfd(__FILE__, "SMUX::~SMUX()\n");
pthread_mutex_destroy(&mutex);
}

int SMUX::ParseSettings()
{
return smuxSettings.ParseSettings(settings);
}

int SMUX::Start()
{
if (PrepareNet())
    return -1;

// Users
sensors[".1.3.6.1.4.1.38313.1.1.1"] = new TotalUsersSensor(*users);
sensors[".1.3.6.1.4.1.38313.1.1.2"] = new ConnectedUsersSensor(*users);
sensors[".1.3.6.1.4.1.38313.1.1.3"] = new AuthorizedUsersSensor(*users);
sensors[".1.3.6.1.4.1.38313.1.1.4"] = new AlwaysOnlineUsersSensor(*users);
sensors[".1.3.6.1.4.1.38313.1.1.5"] = new NoCashUsersSensor(*users);
sensors[".1.3.6.1.4.1.38313.1.1.7"] = new DisabledDetailStatsUsersSensor(*users);
sensors[".1.3.6.1.4.1.38313.1.1.8"] = new DisabledUsersSensor(*users);
sensors[".1.3.6.1.4.1.38313.1.1.9"] = new PassiveUsersSensor(*users);
sensors[".1.3.6.1.4.1.38313.1.1.10"] = new CreditUsersSensor(*users);
sensors[".1.3.6.1.4.1.38313.1.1.11"] = new FreeMbUsersSensor(*users);
sensors[".1.3.6.1.4.1.38313.1.1.12"] = new TariffChangeUsersSensor(*users);
// Tariffs
sensors[".1.3.6.1.4.1.38313.1.2.1"] = new TotalTariffsSensor(*tariffs);

if (!running)
    {
    if (pthread_create(&thread, NULL, Runner, this))
        {
        errorStr = "Cannot create thread.";
        printfd(__FILE__, "Cannot create thread\n");
        return -1;
        }
    }

return 0;
}

int SMUX::Stop()
{
printfd(__FILE__, "SMUX::Stop() - Before\n");
running = false;

if (!stopped)
    {
    //5 seconds to thread stops itself
    for (int i = 0; i < 25 && !stopped; i++)
        {
        struct timespec ts = {0, 200000000};
        nanosleep(&ts, NULL);
        }

    //after 5 seconds waiting thread still running. now killing it
    if (!stopped)
        {
        printfd(__FILE__, "SMUX::Stop() - failed to stop thread, killing it\n");
        if (pthread_kill(thread, SIGINT))
            {
            errorStr = "Cannot kill thread.";
            printfd(__FILE__, "SMUX::Stop() - Cannot kill thread\n");
            return -1;
            }
        printfd(__FILE__, "SMUX::Stop() -  killed Run\n");
        }
    }

pthread_join(thread, NULL);

close(sock);

printfd(__FILE__, "SMUX::Stop() - After\n");
return 0;
}

void * SMUX::Runner(void * d)
{
SMUX * smux = static_cast<SMUX *>(d);

smux->Run();

return NULL;
}

void SMUX::Run()
{
SendOpenPDU(sock);
SendRReqPDU(sock);
running = true;
stopped = false;
while(running)
    {
    if (WaitPackets(sock))
        {
        SMUX_PDUs_t * pdus = RecvSMUXPDUs(sock);
        if (pdus)
            DispatchPDUs(pdus);
        }
    if (!running)
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
    printfd(__FILE__, "Cannot create socket\n");
    return true;
    }

struct sockaddr_in addr;

addr.sin_family = AF_INET;
addr.sin_port = htons(smuxSettings.GetPort());
addr.sin_addr.s_addr = smuxSettings.GetIP();

if (connect(sock, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)))
    {
    errorStr = "Cannot connect.";
    printfd(__FILE__, "Cannot connect. Message: '%s'\n", strerror(errno));
    return true;
    }

return false;
}

bool SMUX::DispatchPDUs(const SMUX_PDUs_t * pdus)
{
SMUXHandlers::iterator it;
it = smuxHandlers.find(pdus->present);
if (it != smuxHandlers.end())
    {
    return (this->*(it->second))(pdus);
    }
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
return false;
}
