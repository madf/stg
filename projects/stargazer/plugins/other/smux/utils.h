#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>

#include "stg/OBJECT_IDENTIFIER.h"
#include "stg/SMUX-PDUs.h"
#include "stg/GetResponse-PDU.h"

bool String2OI(const std::string & str, OBJECT_IDENTIFIER_t * oi);
std::string OI2String(OBJECT_IDENTIFIER_t * oi);
bool SendOpenPDU(int fd);
bool SendClosePDU(int fd);
bool SendRReqPDU(int fd);
SMUX_PDUs_t * RecvSMUXPDUs(int fd);
bool SendGetResponsePDU(int fd, GetResponse_PDU_t * getResponse);
bool SendGetResponseErrorPDU(int fd,
                             const PDU_t * getRequest,
                             int errorStatus,
                             int errorIndex);

#endif
