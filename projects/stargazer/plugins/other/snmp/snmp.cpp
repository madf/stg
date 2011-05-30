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
#include "asn1/SMUX-PDUs.h"
#include "asn1/OBJECT_IDENTIFIER.h"
#include "asn1/ber_decoder.h"
#include "asn1/der_encoder.h"

#include "snmp.h"
#include "stg/common.h"

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

int output(const void * buffer, size_t size, void * data)
{
int * fd = static_cast<int *>(data);
return write(*fd, buffer, size);
}

int SendOpenPDU(int fd)
{
const char * description = "Stg SNMP Agent";
int oid[] = {1, 3, 6, 1, 4, 1, 9363, 1, 5, 2, 1, 1};
asn_enc_rval_t error;
OpenPDU msg;

msg.present = OpenPDU_PR_simple;
asn_long2INTEGER(&msg.choice.simple.version, 1);
OBJECT_IDENTIFIER_set_arcs(&msg.choice.simple.identity,
                           oid,
                           sizeof(oid[0]),
                           sizeof(oid));
OCTET_STRING_fromBuf(&msg.choice.simple.description,
                     description,
                     sizeof(description));
OCTET_STRING_fromBuf(&msg.choice.simple.password,
                     "",
                     0);

error = der_encode(&asn_DEF_OpenPDU, &msg, output, &fd);

if (error.encoded == -1)
    {
    printfd(__FILE__, "Could not encode OpenPDU (at %s)\n",
            error.failed_type ? error.failed_type->name : "unknown");
    return -1;
    }
else
    {
    printfd(__FILE__, "OpenPDU encoded successfully");
    }
return 0;
}

int SendClosePDU(int fd)
{
ClosePDU msg = ClosePDU_goingDown;

asn_enc_rval_t error;
error = der_encode(&asn_DEF_ClosePDU, &msg, output, &fd);

if (error.encoded == -1)
    {
    printfd(__FILE__, "Could not encode ClosePDU (at %s)\n",
            error.failed_type ? error.failed_type->name : "unknown");
    return -1;
    }
else
    {
    printfd(__FILE__, "ClosePDU encoded successfully");
    }
return 0;
}

int RecvSMUXPDUs(int fd)
{
char buffer[8192];
SMUX_PDUs * pdus;

size_t length = read(fd, buffer, sizeof(buffer));
asn_dec_rval_t error;
error = ber_decode(0, &asn_DEF_SMUX_PDUs, (void **)&pdus, buffer, length);
if(error.code != RC_OK)
    {
    printfd(__FILE__, "Failed to decode PDUs at byte %ld\n",
            (long)error.consumed);
    return -1;
    }
switch (pdus->present)
    {
    case SMUX_PDUs_PR_NOTHING:
        printfd(__FILE__, "PDUs: nothing\n");
        break;
    case SMUX_PDUs_PR_open:
        printfd(__FILE__, "PDUs: open\n");
        break;
    case SMUX_PDUs_PR_close:
        printfd(__FILE__, "PDUs: close\n");
        break;
    case SMUX_PDUs_PR_registerRequest:
        printfd(__FILE__, "PDUs: registerRequest\n");
        break;
    case SMUX_PDUs_PR_registerResponse:
        printfd(__FILE__, "PDUs: registerResponse\n");
        break;
    case SMUX_PDUs_PR_pdus:
        printfd(__FILE__, "PDUs: pdus\n");
        break;
    case SMUX_PDUs_PR_commitOrRollback:
        printfd(__FILE__, "PDUs: commitOrRollback\n");
        break;
    default:
        printfd(__FILE__, "PDUs: default\n");
    }
asn_fprint(stderr, &asn_DEF_SMUX_PDUs, pdus);
return 0;
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
}

SNMP_AGENT::~SNMP_AGENT()
{
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
running = false;

close(sock);

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
        if (pthread_kill(thread, SIGINT))
            {
            errorStr = "Cannot kill thread.";
            printfd(__FILE__, "Cannot kill thread\n");
            return -1;
            }
        printfd(__FILE__, "SNMP_AGENT killed Run\n");
        }
    }

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
running = true;
while(running)
    {
    RecvSMUXPDUs(sock);
    struct timespec ts = {1, 0};
    nanosleep(&ts, NULL);
    }
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
