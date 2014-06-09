#include "iface.h"

#include "thriftclient.h"

int stgInstantiateImpl(const char * server, uint16_t port, const char * password)
{
    if (STG_CLIENT_ST::Get().Configure(server, port, password))
        return 1;

    return 0;
}

const STG_PAIR * stgAuthorizeImpl(const char * userName, const char * serviceType)
{
    return STG_CLIENT_ST::Get().Authorize(userName, serviceType);
}

const STG_PAIR * stgAuthenticateImpl(const char * userName, const char * serviceType)
{
    return STG_CLIENT_ST::Get().Authenticate(userName, serviceType);
}

const STG_PAIR * stgPostAuthImpl(const char * userName, const char * serviceType)
{
    return STG_CLIENT_ST::Get().PostAuth(userName, serviceType);
}

/*const STG_PAIR * stgPreAcctImpl(const char * userName, const char * serviceType)
{
    return STG_CLIENT_ST::Get().PreAcct(userName, serviceType);
}*/

const STG_PAIR * stgAccountingImpl(const char * userName, const char * serviceType, const char * statusType, const char * sessionId)
{
    return STG_CLIENT_ST::Get().Account(userName, serviceType, statusType, sessionId);
}

void deletePairs(const STG_PAIR * pairs)
{
    delete[] pairs;
}
