#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <cstring>
#include <cerrno>
#include <ctime>
#include <csignal>

#include <vector>
#include <algorithm>

#include "asn1/OpenPDU.h"
#include "asn1/ClosePDU.h"
#include "asn1/RReqPDU.h"
#include "asn1/OBJECT_IDENTIFIER.h"
#include "asn1/ber_decoder.h"
#include "asn1/der_encoder.h"

#include "snmp.h"
#include "stg/common.h"

bool WaitPackets(int sd);

class SNMP_AGENT_CREATOR
{
private:
    SNMP_AGENT * snmpAgent;

public:
    SNMP_AGENT_CREATOR()
        : snmpAgent(new SNMP_AGENT())
        {
        };
    ~SNMP_AGENT_CREATOR()
        {
        printfd(__FILE__, "SNMP_AGENT_CREATOR::~SNMP_AGENT_CREATOR()\n");
        delete snmpAgent;
        };

    SNMP_AGENT * GetPlugin()
        {
        return snmpAgent;
        };
};

SNMP_AGENT_CREATOR sac;

PLUGIN * GetPlugin()
{
return sac.GetPlugin();
}

int SendOpenPDU(int fd)
{
const char * description = "Stg SNMP Agent";
int oid[] = {1, 3, 6, 1, 4, 1, 9363, 1, 5, 2, 1, 1};
asn_enc_rval_t error;
OpenPDU_t msg;

memset(&msg, 0, sizeof(msg));

msg.present = OpenPDU_PR_simple;
asn_long2INTEGER(&msg.choice.simple.version, SimpleOpen__version_version_1);
OBJECT_IDENTIFIER_set_arcs(&msg.choice.simple.identity,
                           oid,
                           sizeof(oid[0]),
                           7);
OCTET_STRING_fromString(&msg.choice.simple.description,
                     description);
OCTET_STRING_fromString(&msg.choice.simple.password,
                     "");

char buffer[1024];
error = der_encode_to_buffer(&asn_DEF_OpenPDU, &msg, buffer, sizeof(buffer));

if (error.encoded == -1)
    {
    printfd(__FILE__, "Could not encode OpenPDU (at %s)\n",
            error.failed_type ? error.failed_type->name : "unknown");
    return -1;
    }
else
    {
    write(fd, buffer, error.encoded);
    printfd(__FILE__, "OpenPDU encoded successfully to %d bytes\n", error.encoded);
    }
return 0;
}

int SendClosePDU(int fd)
{
ClosePDU_t msg;

memset(&msg, 0, sizeof(msg));

asn_long2INTEGER(&msg, ClosePDU_goingDown);

char buffer[1024];
asn_enc_rval_t error;
error = der_encode_to_buffer(&asn_DEF_ClosePDU, &msg, buffer, sizeof(buffer));

if (error.encoded == -1)
    {
    printfd(__FILE__, "Could not encode ClosePDU (at %s)\n",
            error.failed_type ? error.failed_type->name : "unknown");
    return -1;
    }
else
    {
    write(fd, buffer, error.encoded);
    printfd(__FILE__, "ClosePDU encoded successfully\n");
    }
return 0;
}

int SendRReqPDU(int fd)
{
int oid[] = {1, 3, 6, 1, 4, 1, 9363, 1};
asn_enc_rval_t error;
RReqPDU_t msg;

memset(&msg, 0, sizeof(msg));

msg.priority = 0;
asn_long2INTEGER(&msg.operation, RReqPDU__operation_readOnly);
OBJECT_IDENTIFIER_set_arcs(&msg.subtree,
                           oid,
                           sizeof(oid[0]),
                           8);

char buffer[1024];
error = der_encode_to_buffer(&asn_DEF_RReqPDU, &msg, buffer, sizeof(buffer));

if (error.encoded == -1)
    {
    printfd(__FILE__, "Could not encode RReqPDU (at %s)\n",
            error.failed_type ? error.failed_type->name : "unknown");
    return -1;
    }
else
    {
    write(fd, buffer, error.encoded);
    printfd(__FILE__, "RReqPDU encoded successfully to %d bytes\n", error.encoded);
    }
return 0;
}

SMUX_PDUs_t * RecvSMUXPDUs(int fd)
{
char buffer[1024];
SMUX_PDUs_t * pdus = NULL;

memset(buffer, 0, sizeof(buffer));

size_t length = read(fd, buffer, sizeof(buffer));
if (length < 1)
    return NULL;
asn_dec_rval_t error;
error = ber_decode(0, &asn_DEF_SMUX_PDUs, (void **)&pdus, buffer, length);
if(error.code != RC_OK)
    {
    printfd(__FILE__, "Failed to decode PDUs at byte %ld\n",
            (long)error.consumed);
    return NULL;
    }
return pdus;
}

int ParseIntInRange(const std::string & str,
                    int min,
                    int max,
                    int * val)
{
if (str2x(str.c_str(), *val))
    {
    return -1;
    }
if (*val < min || *val > max)
    {
    return -1;
    }
return 0;
}

SNMP_AGENT_SETTINGS::SNMP_AGENT_SETTINGS()
    : ip(0),
      port(0)
{}

int SNMP_AGENT_SETTINGS::ParseSettings(const MODULE_SETTINGS & s)
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

SNMP_AGENT::SNMP_AGENT()
    : PLUGIN(),
      running(false),
      stopped(true),
      sock(-1)
{
pthread_mutex_init(&mutex, NULL);

handlers[SMUX_PDUs_PR_close] = &SNMP_AGENT::CloseHandler;
handlers[SMUX_PDUs_PR_registerResponse] = &SNMP_AGENT::RegisterResponseHandler;
handlers[SMUX_PDUs_PR_pdus] = &SNMP_AGENT::PDUsHandler;
handlers[SMUX_PDUs_PR_commitOrRollback] = &SNMP_AGENT::CommitOrRollbackHandler;
}

SNMP_AGENT::~SNMP_AGENT()
{
printfd(__FILE__, "SNMP_AGENT::~SNMP_AGENT()\n");
pthread_mutex_destroy(&mutex);
}

int SNMP_AGENT::ParseSettings()
{
return snmpAgentSettings.ParseSettings(settings);
}

int SNMP_AGENT::Start()
{
if (PrepareNet())
    return -1;

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

int SNMP_AGENT::Stop()
{
printfd(__FILE__, "SNMP_AGENT::Stop() - Before\n");
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
        printfd(__FILE__, "SNMP_AGENT::Stop() - failed to stop thread, killing it\n");
        if (pthread_kill(thread, SIGINT))
            {
            errorStr = "Cannot kill thread.";
            printfd(__FILE__, "SNMP_AGENT::Stop() - Cannot kill thread\n");
            return -1;
            }
        printfd(__FILE__, "SNMP_AGENT::Stop() -  killed Run\n");
        }
    }

pthread_join(thread, NULL);

close(sock);

printfd(__FILE__, "SNMP_AGENT::Stop() - After\n");
return 0;
}

void * SNMP_AGENT::Runner(void * d)
{
SNMP_AGENT * snmpAgent = static_cast<SNMP_AGENT *>(d);

snmpAgent->Run();

return NULL;
}

void SNMP_AGENT::Run()
{
SendOpenPDU(sock);
SendRReqPDU(sock);
running = true;
stopped = false;
printfd(__FILE__, "SNMP_AGENT::Run() - Before\n");
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
printfd(__FILE__, "SNMP_AGENT::Run() - After\n");
SendClosePDU(sock);
stopped = true;
}

bool SNMP_AGENT::PrepareNet()
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
addr.sin_port = htons(snmpAgentSettings.GetPort());
addr.sin_addr.s_addr = snmpAgentSettings.GetIP();

if (connect(sock, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)))
    {
    errorStr = "Cannot connect.";
    printfd(__FILE__, "Cannot connect. Message: '%s'\n", strerror(errno));
    return true;
    }

return false;
}

bool WaitPackets(int sd)
{
fd_set rfds;
FD_ZERO(&rfds);
FD_SET(sd, &rfds);

struct timeval tv;
tv.tv_sec = 0;
tv.tv_usec = 500000;

int res = select(sd + 1, &rfds, NULL, NULL, &tv);
if (res == -1) // Error
    {
    if (errno != EINTR)
        {
        printfd(__FILE__, "Error on select: '%s'\n", strerror(errno));
        }
    return false;
    }

if (res == 0) // Timeout
    {
    return false;
    }

return true;
}

bool SNMP_AGENT::DispatchPDUs(const SMUX_PDUs_t * pdus)
{
std::map<SMUX_PDUs_PR, SNMPPacketHandler>::iterator it;
it = handlers.find(pdus->present);
if (it != handlers.end())
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

bool SNMP_AGENT::CloseHandler(const SMUX_PDUs_t * pdus)
{
printfd(__FILE__, "SNMP_AGENT::CloseHandler()\n");
asn_fprint(stderr, &asn_DEF_SMUX_PDUs, pdus);
return false;
}

bool SNMP_AGENT::RegisterResponseHandler(const SMUX_PDUs_t * pdus)
{
printfd(__FILE__, "SNMP_AGENT::RegisterResponseHandler()\n");
asn_fprint(stderr, &asn_DEF_SMUX_PDUs, pdus);
return false;
}

bool SNMP_AGENT::PDUsHandler(const SMUX_PDUs_t * pdus)
{
printfd(__FILE__, "SNMP_AGENT::PDUsHandler()\n");
asn_fprint(stderr, &asn_DEF_SMUX_PDUs, pdus);
return false;
}

bool SNMP_AGENT::CommitOrRollbackHandler(const SMUX_PDUs_t * pdus)
{
printfd(__FILE__, "SNMP_AGENT::CommitOrRollbackHandler()\n");
asn_fprint(stderr, &asn_DEF_SMUX_PDUs, pdus);
return false;
}
