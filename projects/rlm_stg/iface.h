#ifndef __STG_IFACE_H__
#define __STG_IFACE_H__

#include <stdint.h>

#include "stgpair.h"

#ifdef __cplusplus
extern "C" {
#endif

int stgInstantiateImpl(const char * server, uint16_t port, const char * password);
const STG_PAIR * stgAuthorizeImpl(const char * userName, const char * serviceType);
const STG_PAIR * stgAuthenticateImpl(const char * userName, const char * serviceType);
const STG_PAIR * stgPostAuthImpl(const char * userName, const char * serviceType);
/*const STG_PAIR * stgPreAcctImpl(const char * userName, const char * serviceType);*/
const STG_PAIR * stgAccountingImpl(const char * userName, const char * serviceType, const char * statusType, const char * sessionId);

void deletePairs(const STG_PAIR * pairs);

#ifdef __cplusplus
}
#endif

#endif
