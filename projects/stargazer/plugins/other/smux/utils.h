#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include "stg/OBJECT_IDENTIFIER.h"
#include "stg/SMUX-PDUs.h"
#include "stg/GetResponse-PDU.h"
#pragma GCC diagnostic pop

#include <string>

bool String2OI(const std::string & str, OBJECT_IDENTIFIER_t * oi);
bool SendOpenPDU(int fd);
bool SendClosePDU(int fd);
bool SendRReqPDU(int fd);
SMUX_PDUs_t * RecvSMUXPDUs(int fd);
bool SendGetResponsePDU(int fd, GetResponse_PDU_t * getResponse);
bool SendGetResponseErrorPDU(int fd,
                             const PDU_t * getRequest,
                             int errorStatus,
                             int errorIndex);
