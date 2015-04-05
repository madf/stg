#include "iface.h"

#include "stg_client.h"

namespace
{

STG_PAIR* toPairs(const PAIRS& source)
{
    STG_PAIR * pairs = new STG_PAIR[source.size() + 1];
    for (size_t pos = 0; pos < source.size(); ++pos) {
        bzero(pairs[pos].key, sizeof(STG_PAIR::key));
        bzero(pairs[pos].value, sizeof(STG_PAIR::value));
        strncpy(pairs[pos].key, source[pos].first.c_str(), sizeof(STG_PAIR::key));
        strncpy(pairs[pos].value, source[pos].second.c_str(), sizeof(STG_PAIR::value));
        ++pos;
    }
    bzero(pairs[sources.size()].key, sizeof(STG_PAIR::key));
    bzero(pairs[sources.size()].value, sizeof(STG_PAIR::value));

    return pairs;
}

}

int stgInstantiateImpl(const char* server, uint16_t port, const char* password)
{
    if (STG_CLIENT::configure(server, port, password))
        return 1;

    return 0;
}

const STG_PAIR* stgAuthorizeImpl(const char* userName, const char* serviceType)
{
    STG_CLIENT* client = STG_CLIENT::get();
    if (client == NULL) {
        // TODO: log "Not configured"
        return NULL;
    }
    return toPairs(client->authorize(userName, serviceType));
}

const STG_PAIR* stgAuthenticateImpl(const char* userName, const char* serviceType)
{
    STG_CLIENT* client = STG_CLIENT::get();
    if (client == NULL) {
        // TODO: log "Not configured"
        return NULL;
    }
    return toPairs(client->authenticate(userName, serviceType));
}

const STG_PAIR* stgPostAuthImpl(const char* userName, const char* serviceType)
{
    STG_CLIENT* client = STG_CLIENT::get();
    if (client == NULL) {
        // TODO: log "Not configured"
        return NULL;
    }
    return toPairs(client->postAuth(userName, serviceType));
}

const STG_PAIR* stgPreAcctImpl(const char* userName, const char* serviceType)
{
    STG_CLIENT* client = STG_CLIENT::get();
    if (client == NULL) {
        // TODO: log "Not configured"
        return NULL;
    }
    return toPairs(client->preAcct(userName, serviceType));
}

const STG_PAIR* stgAccountingImpl(const char* userName, const char* serviceType, const char* statusType, const char* sessionId)
{
    STG_CLIENT* client = STG_CLIENT::get();
    if (client == NULL) {
        // TODO: log "Not configured"
        return NULL;
    }
    return toPairs(client->account(userName, serviceType, statusType, sessionId));
}

int countValuePairs(const VALUE_PAIR* pairs)
{
    unsigned count = 0;
    while (pairs != NULL) {
        ++count;
        pairs = pairs->next;
    }
    return count;
}

STG_PAIR* fromValuePairs(const VALUE_PAIR* pairs)
{
    unsigned size = countValuePairs(pairs);
    STG_PAIR* res = new STG_PAIR[size + 1];
    size_t pos = 0;
    while (pairs != NULL) {
        bzero(res[pos].key, sizeof(STG_PAIR::key));
        bzero(res[pos].value, sizeof(STG_PAIR::value));
        strncpy(res[pos].key, pairs->name, sizeof(STG_PAIR::key));
        strncpy(res[pos].value, pairs->data.strvalue, sizeof(STG_PAIR::value));
        ++pos;
        pairs = pairs->next;
    }
    bzero(res[pos].key, sizeof(STG_PAIR::key));
    bzero(res[pos].value, sizeof(STG_PAIR::value));
    return res;
}

void deletePairs(const STG_PAIR* pairs)
{
    delete[] pairs;
}
