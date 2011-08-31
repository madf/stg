#include <unistd.h> // write

#include <cstring> // memset
#include <cerrno>

#include "stg/common.h"

#include "stg/OpenPDU.h"
#include "stg/ClosePDU.h"
#include "stg/RReqPDU.h"
#include "stg/ber_decoder.h"
#include "stg/der_encoder.h"

#include "pen.h"
#include "utils.h"

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

bool SendOpenPDU(int fd)
{
const char * description = "Stg SMUX Plugin";
asn_enc_rval_t error;
OpenPDU_t msg;

memset(&msg, 0, sizeof(msg));

msg.present = OpenPDU_PR_simple;
asn_long2INTEGER(&msg.choice.simple.version, SimpleOpen__version_version_1);
if (!String2OI(PEN_PREFIX, &msg.choice.simple.identity))
    {
    printfd(__FILE__,
            "SendOpenPDU() - failed to convert string to OBJECT_IDENTIFIER\n");
    return false;
    }
OCTET_STRING_fromString(&msg.choice.simple.description, description);
OCTET_STRING_fromString(&msg.choice.simple.password, "");

char buffer[1024];
error = der_encode_to_buffer(&asn_DEF_OpenPDU, &msg, buffer, sizeof(buffer));

ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_OpenPDU, &msg);

if (error.encoded == -1)
    {
    printfd(__FILE__, "Could not encode OpenPDU (at %s)\n",
            error.failed_type ? error.failed_type->name : "unknown");
    return false;
    }
else
    {
    write(fd, buffer, error.encoded);
    printfd(__FILE__, "OpenPDU encoded successfully to %d bytes\n",
            error.encoded);
    }
return true;
}

int SendClosePDU(int fd)
{
ClosePDU_t msg;

memset(&msg, 0, sizeof(msg));

asn_long2INTEGER(&msg, ClosePDU_goingDown);

char buffer[1024];
asn_enc_rval_t error;
error = der_encode_to_buffer(&asn_DEF_ClosePDU, &msg, buffer, sizeof(buffer));

ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_ClosePDU, &msg);

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

ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_RReqPDU, &msg);

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

long id = 0;
asn_INTEGER2long(&getRequest->request_id, &id);
asn_long2INTEGER(&msg.request_id, id);
asn_long2INTEGER(&msg.error_status, errorStatus);
asn_long2INTEGER(&msg.error_index, errorIndex);

char buffer[1024];
error = der_encode_to_buffer(&asn_DEF_GetResponse_PDU, &msg, buffer,
                             sizeof(buffer));

ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_GetResponse_PDU, &msg);

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
