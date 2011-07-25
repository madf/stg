#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>

#include "asn1/OBJECT_IDENTIFIER.h"
#include "SMUX-PDUs.h"
#include "asn1/GetResponse-PDU.h"

bool WaitPackets(int sd);
bool String2OI(const std::string & str, OBJECT_IDENTIFIER_t * oi);
std::string OI2String(OBJECT_IDENTIFIER_t * oi);
bool SendOpenPDU(int fd);
int SendClosePDU(int fd);
int SendRReqPDU(int fd);
SMUX_PDUs_t * RecvSMUXPDUs(int fd);
int SendGetResponsePDU(int fd, GetResponse_PDU_t * getResponse);
int SendGetResponseErrorPDU(int fd,
                            const PDU_t * getRequest,
                            int errorStatus,
                            int errorIndex);

#endif
