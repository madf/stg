#include <cassert>

#include "stg/GetRequest-PDU.h"
#include "stg/GetResponse-PDU.h"
#include "stg/VarBindList.h"
#include "stg/VarBind.h"

#include "stg/common.h"

#include "utils.h"
#include "smux.h"

#ifdef SMUX_DEBUG
bool SMUX::CloseHandler(const SMUX_PDUs_t * pdus)
{
printfd(__FILE__, "SMUX::CloseHandler()\n");
asn_fprint(stderr, &asn_DEF_SMUX_PDUs, pdus);
return true;
}
#else
bool SMUX::CloseHandler(const SMUX_PDUs_t *)
{
return true;
}
#endif

#ifdef SMUX_DEBUG
bool SMUX::RegisterResponseHandler(const SMUX_PDUs_t * pdus)
{
printfd(__FILE__, "SMUX::RegisterResponseHandler()\n");
asn_fprint(stderr, &asn_DEF_SMUX_PDUs, pdus);
return true;
}
#else
bool SMUX::RegisterResponseHandler(const SMUX_PDUs_t *)
{
return true;
}
#endif

bool SMUX::PDUsRequestHandler(const SMUX_PDUs_t * pdus)
{
#ifdef SMUX_DEBUG
printfd(__FILE__, "SMUX::PDUsRequestHandler()\n");
asn_fprint(stderr, &asn_DEF_SMUX_PDUs, pdus);
#endif
PDUsHandlers::iterator it(pdusHandlers.find(pdus->choice.pdus.present));
if (it != pdusHandlers.end())
    {
    return (this->*(it->second))(&pdus->choice.pdus);
    }
#ifdef SMUX_DEBUG
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
#endif
return true;
}

#ifdef SMUX_DEBUG
bool SMUX::CommitOrRollbackHandler(const SMUX_PDUs_t * pdus)
{
printfd(__FILE__, "SMUX::CommitOrRollbackHandler()\n");
asn_fprint(stderr, &asn_DEF_SMUX_PDUs, pdus);
return true;
}
#else
bool SMUX::CommitOrRollbackHandler(const SMUX_PDUs_t *)
{
return true;
}
#endif

bool SMUX::GetRequestHandler(const PDUs_t * pdus)
{
#ifdef SMUX_DEBUG
printfd(__FILE__, "SMUX::GetRequestHandler()\n");
asn_fprint(stderr, &asn_DEF_PDUs, pdus);
#endif
const GetRequest_PDU_t * getRequest = &pdus->choice.get_request;
GetResponse_PDU_t * msg = static_cast<GetResponse_PDU_t *>(calloc(1, sizeof(GetResponse_PDU_t)));
assert(msg && "Enought mempry to allocate GetResponse_PDU_t");
VarBindList_t * varBindList = &msg->variable_bindings;

long id = 0;
asn_INTEGER2long(&getRequest->request_id, &id);
asn_long2INTEGER(&msg->request_id, id);
asn_long2INTEGER(&msg->error_status, 0);
asn_long2INTEGER(&msg->error_index, 0);

const VarBindList_t * vbl = &getRequest->variable_bindings;
for (int i = 0; i < vbl->list.count; ++i)
    {
    VarBind_t * vb = getRequest->variable_bindings.list.array[i];
    Sensors::iterator it;
    it = sensors.find(OID(&vb->name));
    if (it == sensors.end())
        {
        return SendGetResponseErrorPDU(sock, getRequest,
                                       PDU__error_status_noSuchName, i);
        }

    VarBind_t * newVb = static_cast<VarBind_t *>(calloc(1, sizeof(VarBind_t)));
    assert(newVb && "Enought mempry to allocate VarBind_t");

    it->first.ToOID(&newVb->name);
    it->second->GetValue(&newVb->value);

    ASN_SEQUENCE_ADD(varBindList, newVb);
    }

bool res = SendGetResponsePDU(sock, msg);
#ifdef SMUX_DEBUG
asn_fprint(stderr, &asn_DEF_GetResponse_PDU, msg);
#endif
ASN_STRUCT_FREE(asn_DEF_GetResponse_PDU, msg);
return res;
}

bool SMUX::GetNextRequestHandler(const PDUs_t * pdus)
{
#ifdef SMUX_DEBUG
printfd(__FILE__, "SMUX::GetNextRequestHandler()\n");
asn_fprint(stderr, &asn_DEF_PDUs, pdus);
#endif
const GetRequest_PDU_t * getRequest = &pdus->choice.get_request;
GetResponse_PDU_t * msg = static_cast<GetResponse_PDU_t *>(calloc(1, sizeof(GetResponse_PDU_t)));
assert(msg && "Enought mempry to allocate GetResponse_PDU_t");
VarBindList_t * varBindList = &msg->variable_bindings;

long id = 0;
asn_INTEGER2long(&getRequest->request_id, &id);
asn_long2INTEGER(&msg->request_id, id);
asn_long2INTEGER(&msg->error_status, 0);
asn_long2INTEGER(&msg->error_index, 0);

const VarBindList_t * vbl = &getRequest->variable_bindings;
for (int i = 0; i < vbl->list.count; ++i)
    {
    VarBind_t * vb = getRequest->variable_bindings.list.array[i];
    Sensors::iterator it;
    it = sensors.upper_bound(OID(&vb->name));
    if (it == sensors.end())
        {
#ifdef SMUX_DEBUG
        printfd(__FILE__, "SMUX::GetNextRequestHandler() - '%s' not found\n", OID(&vb->name).ToString().c_str());
#endif
        return SendGetResponseErrorPDU(sock, getRequest,
                                       PDU__error_status_noSuchName, i);
        }

    VarBind_t * newVb = static_cast<VarBind_t *>(calloc(1, sizeof(VarBind_t)));
    assert(newVb && "Enought mempry to allocate VarBind_t");

    it->first.ToOID(&newVb->name);
    it->second->GetValue(&newVb->value);

    ASN_SEQUENCE_ADD(varBindList, newVb);
    }

bool res = SendGetResponsePDU(sock, msg);
#ifdef SMUX_DEBUG
asn_fprint(stderr, &asn_DEF_PDU, msg);
#endif
ASN_STRUCT_FREE(asn_DEF_GetResponse_PDU, msg);
return res;
}

bool SMUX::SetRequestHandler(const PDUs_t * pdus)
{
#ifdef SMUX_DEBUG
printfd(__FILE__, "SMUX::SetRequestHandler()\n");
asn_fprint(stderr, &asn_DEF_PDUs, pdus);
#endif
return SendGetResponseErrorPDU(sock, &pdus->choice.set_request,
                               PDU__error_status_readOnly, 0);
}
