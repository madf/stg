#ifndef __STG_IFACE_H__
#define __STG_IFACE_H__

#include <stdint.h>

#include "stgpair.h"

#ifdef __cplusplus
extern "C" {
#endif

int stgInstantiateImpl(const char* address);
STG_RESULT stgAuthorizeImpl(const char* userName, const char* password, const STG_PAIR* vps);
STG_RESULT stgAuthenticateImpl(const char* userName, const char* password, const STG_PAIR* vps);
STG_RESULT stgPostAuthImpl(const char* userName, const char* password, const STG_PAIR* vps);
STG_RESULT stgPreAcctImpl(const char* userName, const char* password, const STG_PAIR* vps);
STG_RESULT stgAccountingImpl(const char* userName, const char* password, const STG_PAIR* vps);

#ifdef __cplusplus
}
#endif

#endif
