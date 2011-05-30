#include "asn1/OpenPDU.h"

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

SNMP_AGENT_CREATOR pc;

int output(const void * buffer, size_t size, void * data)
{
int * fd = static_cast<int *>(data);
return write(*fd, buffer, size);
}

int SendOpenPDU(int fd, OpenPDU & msg)
{
asn_enc_rval_t error;

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

SNMP_AGENT::SNMP_AGENT()
    : PLUGIN(),
      running(false),
      stopped(true)
{
pthread_mutex_init(&mutex, NULL);
}

SNMP_AGENT::~SNMP_AGENT()
{
pthread_mutex_destroy(&mutex);
}

int SNMP_AGENT::ParseSettings()
{
return 0;
}

int SNMP_AGENT::Start()
{
return 0;
}

int SNMP_AGENT::Stop()
{
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
}
