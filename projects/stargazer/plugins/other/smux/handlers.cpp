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

std::string OI2String(OBJECT_IDENTIFIER_t * oi);
int SendGetResponsePDU(int fd, GetResponse_PDU_t * getResponse);
int SendGetResponseErrorPDU(int fd,
                            const PDU_t * getRequest,
                            int errorStatus,
                            int errorIndex);

bool SMUX::CloseHandler(const SMUX_PDUs_t * pdus)
{
printfd(__FILE__, "SMUX::CloseHandler()\n");
asn_fprint(stderr, &asn_DEF_SMUX_PDUs, pdus);
return false;
}

bool SMUX::RegisterResponseHandler(const SMUX_PDUs_t * pdus)
{
printfd(__FILE__, "SMUX::RegisterResponseHandler()\n");
asn_fprint(stderr, &asn_DEF_SMUX_PDUs, pdus);
return false;
}

bool SMUX::PDUsRequestHandler(const SMUX_PDUs_t * pdus)
{
printfd(__FILE__, "SMUX::PDUsRequestHandler()\n");
asn_fprint(stderr, &asn_DEF_SMUX_PDUs, pdus);
PDUsHandlers::iterator it;
it = pdusHandlers.find(pdus->choice.pdus.present);
if (it != pdusHandlers.end())
    {
    return (this->*(it->second))(&pdus->choice.pdus);
    }
else
    {
    switch (pdus->present)
        {
        case PDUs_PR_NOTHING:
            printfd(__FILE__, "SMUX::PDUsRequestHandler() - nothing\n");
            break;
        case PDUs_PR_get_response:
            printfd(__FILE__, "SMUX::PDUsRequestHandler() - get response\n");
            break;
        case PDUs_PR_trap:
            printfd(__FILE__, "SMUX::PDUsRequestHandler() - trap\n");
            break;
        default:
            printfd(__FILE__, "SMUX::PDUsRequestHandler() - undefined\n");
        }
    }
return false;
}

bool SMUX::CommitOrRollbackHandler(const SMUX_PDUs_t * pdus)
{
printfd(__FILE__, "SMUX::CommitOrRollbackHandler()\n");
asn_fprint(stderr, &asn_DEF_SMUX_PDUs, pdus);
return false;
}

bool SMUX::GetRequestHandler(const PDUs_t * pdus)
{
printfd(__FILE__, "SMUX::GetRequestHandler()\n");
asn_fprint(stderr, &asn_DEF_PDUs, pdus);
const GetRequest_PDU_t * getRequest = &pdus->choice.get_request;
GetResponse_PDU_t msg;
VarBindList_t * varBindList = &msg.variable_bindings;
memset(&msg, 0, sizeof(msg));

msg.request_id = getRequest->request_id;
asn_long2INTEGER(&msg.error_status, 0);
asn_long2INTEGER(&msg.error_index, 0);

const VarBindList_t * vbl = &getRequest->variable_bindings; 
for (int i = 0; i < vbl->list.count; ++i)
    {
    VarBind_t * vb = getRequest->variable_bindings.list.array[i];
    Sensors::iterator it;
    it = sensors.find(OI2String(&vb->name));
    if (it == sensors.end())
        {
        SendGetResponseErrorPDU(sock, getRequest,
                                PDU__error_status_noSuchName, i);
        return true;
        }

    VarBind_t newVb;
    memset(&newVb, 0, sizeof(newVb));

    newVb.name = vb->name;
    it->second->GetValue(&newVb.value);

    ASN_SEQUENCE_ADD(varBindList, &newVb);
    }

SendGetResponsePDU(sock, &msg);
asn_fprint(stderr, &asn_DEF_PDU, &msg);
return false;
}

bool SMUX::GetNextRequestHandler(const PDUs_t * pdus)
{
printfd(__FILE__, "SMUX::GetNextRequestHandler()\n");
asn_fprint(stderr, &asn_DEF_PDUs, pdus);
const GetRequest_PDU_t * getRequest = &pdus->choice.get_request;
GetResponse_PDU_t msg;
VarBindList_t * varBindList = &msg.variable_bindings;
memset(&msg, 0, sizeof(msg));

msg.request_id = getRequest->request_id;
asn_long2INTEGER(&msg.error_status, 0);
asn_long2INTEGER(&msg.error_index, 0);

const VarBindList_t * vbl = &getRequest->variable_bindings; 
for (int i = 0; i < vbl->list.count; ++i)
    {
    VarBind_t * vb = getRequest->variable_bindings.list.array[i];
    Sensors::iterator it;
    it = sensors.upper_bound(OI2String(&vb->name));
    if (it == sensors.end())
        {
        SendGetResponseErrorPDU(sock, getRequest,
                                PDU__error_status_noSuchName, i);
        return true;
        }

    VarBind_t newVb;
    memset(&newVb, 0, sizeof(newVb));

    newVb.name = vb->name;
    it->second->GetValue(&newVb.value);

    ASN_SEQUENCE_ADD(varBindList, &newVb);
    }

SendGetResponsePDU(sock, &msg);
asn_fprint(stderr, &asn_DEF_PDU, &msg);
return false;
}

bool SMUX::SetRequestHandler(const PDUs_t * pdus)
{
printfd(__FILE__, "SMUX::SetRequestHandler()\n");
asn_fprint(stderr, &asn_DEF_PDUs, pdus);
SendGetResponseErrorPDU(sock, &pdus->choice.set_request,
                        PDU__error_status_readOnly, 0);
return false;
}

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
