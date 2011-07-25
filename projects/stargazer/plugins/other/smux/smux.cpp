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
#include "asn1/GetRequest-PDU.h"
#include "asn1/GetResponse-PDU.h"
#include "asn1/VarBindList.h"
#include "asn1/VarBind.h"
#include "asn1/OBJECT_IDENTIFIER.h"
#include "asn1/ber_decoder.h"
#include "asn1/der_encoder.h"

#include "stg/common.h"

#include "smux.h"

bool WaitPackets(int sd);

std::string OI2String(OBJECT_IDENTIFIER_t * oi)
{
std::string res;

int arcs[1024];
int count = OBJECT_IDENTIFIER_get_arcs(oi, arcs, sizeof(arcs[0]), 1024);

if (count > 1024)
    return "";

for (int i = 0; i < count; ++i)
    {
    res += ".";
    std::string arc;
    strprintf(&arc, "%d", arcs[i]);
    res += arc;
    }

return res;
}

bool String2OI(const std::string & str, OBJECT_IDENTIFIER_t * oi)
{
size_t left = 0, pos = 0, arcPos = 0;
int arcs[1024];
pos = str.find_first_of('.', left);
if (pos == 0)
    {
    left = 1;
    pos = str.find_first_of('.', left);
    }
while (pos != std::string::npos)
    {
    int arc = 0;
    if (str2x(str.substr(left, left - pos), arc))
        {
        return false;
        }
    arcs[arcPos++] = arc;
    left = pos + 1;
    pos = str.find_first_of('.', left);
    }
if (left < str.length())
    {
    int arc = 0;
    if (str2x(str.substr(left, left - pos), arc))
        {
        return false;
        }
    arcs[arcPos++] = arc;
    }
printfd(__FILE__, "String2OI() - arcPos: %d\n", arcPos);
OBJECT_IDENTIFIER_set_arcs(oi, arcs, sizeof(arcs[0]), arcPos);
return true;
}

class SMUX_CREATOR
{
private:
    SMUX * smux;

public:
    SMUX_CREATOR() : smux(new SMUX()) {}
    ~SMUX_CREATOR() { delete smux; }

    SMUX * GetPlugin() { return smux; }
};

SMUX_CREATOR sac;

PLUGIN * GetPlugin()
{
return sac.GetPlugin();
}

int SendOpenPDU(int fd)
{
const char * description = "Stg SMUX Plugin";
//int oid[] = {1, 3, 6, 1, 4, 1, 38313, 1, 5, 2, 1, 1};
asn_enc_rval_t error;
OpenPDU_t msg;

memset(&msg, 0, sizeof(msg));

msg.present = OpenPDU_PR_simple;
asn_long2INTEGER(&msg.choice.simple.version, SimpleOpen__version_version_1);
if (!String2OI(".1.3.6.1.4.1.38313", &msg.choice.simple.identity))
    {
    printfd(__FILE__,
            "SendOpenPDU() - failed to convert string to OBJECT_IDENTIFIER\n");
    return -1;
    }
OCTET_STRING_fromString(&msg.choice.simple.description, description);
OCTET_STRING_fromString(&msg.choice.simple.password, "");

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
    printfd(__FILE__, "OpenPDU encoded successfully to %d bytes\n",
            error.encoded);
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
int oid[] = {1, 3, 6, 1, 4, 1, 38313, 1};
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
    printfd(__FILE__, "RReqPDU encoded successfully to %d bytes\n",
            error.encoded);
    }
return 0;
}

int SendGetResponsePDU(int fd, GetResponse_PDU_t * getResponse)
{
asn_enc_rval_t error;

char buffer[1024];
error = der_encode_to_buffer(&asn_DEF_GetResponse_PDU, getResponse, buffer,
                             sizeof(buffer));

if (error.encoded == -1)
    {
    printfd(__FILE__, "Could not encode GetResponsePDU (at %s)\n",
            error.failed_type ? error.failed_type->name : "unknown");
    return -1;
    }
else
    {
    write(fd, buffer, error.encoded);
    printfd(__FILE__, "GetResponsePDU encoded successfully to %d bytes\n",
            error.encoded);
    }
return 0;
}

int SendGetResponseErrorPDU(int fd,
                            const PDU_t * getRequest,
                            int errorStatus,
                            int errorIndex)
{
asn_enc_rval_t error;
GetResponse_PDU_t msg;

memset(&msg, 0, sizeof(msg));

msg.request_id = getRequest->request_id;
asn_long2INTEGER(&msg.error_status, errorStatus);
asn_long2INTEGER(&msg.error_index, errorIndex);

char buffer[1024];
error = der_encode_to_buffer(&asn_DEF_GetResponse_PDU, &msg, buffer,
                             sizeof(buffer));

if (error.encoded == -1)
    {
    printfd(__FILE__, "Could not encode GetResponsePDU for error (at %s)\n",
            error.failed_type ? error.failed_type->name : "unknown");
    return -1;
    }
else
    {
    write(fd, buffer, error.encoded);
    printfd(__FILE__,
            "GetResponsePDU for error encoded successfully to %d bytes\n",
            error.encoded);
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
