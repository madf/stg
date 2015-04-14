#include "iface.h"

#include "stg_client.h"

#include <cstring>

#include <strings.h>

namespace
{

STG_PAIR* toSTGPairs(const PAIRS& source)
{
    STG_PAIR * pairs = new STG_PAIR[source.size() + 1];
    for (size_t pos = 0; pos < source.size(); ++pos) {
        bzero(pairs[pos].key, sizeof(STG_PAIR::key));
        bzero(pairs[pos].value, sizeof(STG_PAIR::value));
        strncpy(pairs[pos].key, source[pos].first.c_str(), sizeof(STG_PAIR::key));
        strncpy(pairs[pos].value, source[pos].second.c_str(), sizeof(STG_PAIR::value));
        ++pos;
    }
    bzero(pairs[source.size()].key, sizeof(STG_PAIR::key));
    bzero(pairs[source.size()].value, sizeof(STG_PAIR::value));

    return pairs;
}

PAIRS fromSTGPairs(const STG_PAIR* pairs)
{
    const STG_PAIR* pair = pairs;
    PAIRS res;

    while (!emptyPair(pair)) {
        res.push_back(std::pair<std::string, std::string>(pair->key, pair->value));
        ++pair;
    }

    return res;
}

STG_RESULT toResult(const RESULT& source)
{
    STG_RESULT result;
    result.modify = toSTGPairs(source.modify);
    result.reply = toSTGPairs(source.reply);
    return result;
}

STG_RESULT emptyResult()
{
    STG_RESULT result = {NULL, NULL};
    return result;
}

std::string toString(const char* value)
{
    if (value == NULL)
        return "";
    else
        return value;
}

STG_RESULT stgRequest(STG_CLIENT::TYPE type, const char* userName, const char* password, const STG_PAIR* pairs)
{
    STG_CLIENT* client = STG_CLIENT::get();
    if (client == NULL) {
        // TODO: log "Not configured"
        return emptyResult();
    }
    try {
        return toResult(client->request(type, toString(userName), toString(password), fromSTGPairs(pairs)));
    } catch (const STG_CLIENT::Error& ex) {
        // TODO: log error
        return emptyResult();
    }
}

}

int stgInstantiateImpl(const char* address)
{
    if (STG_CLIENT::configure(toString(address)))
        return 1;

    return 0;
}

STG_RESULT stgAuthorizeImpl(const char* userName, const char* password, const STG_PAIR* pairs)
{
    return stgRequest(STG_CLIENT::AUTHORIZE, userName, password, pairs);
}

STG_RESULT stgAuthenticateImpl(const char* userName, const char* password, const STG_PAIR* pairs)
{
    return stgRequest(STG_CLIENT::AUTHENTICATE, userName, password, pairs);
}

STG_RESULT stgPostAuthImpl(const char* userName, const char* password, const STG_PAIR* pairs)
{
    return stgRequest(STG_CLIENT::POST_AUTH, userName, password, pairs);
}

STG_RESULT stgPreAcctImpl(const char* userName, const char* password, const STG_PAIR* pairs)
{
    return stgRequest(STG_CLIENT::PRE_ACCT, userName, password, pairs);
}

STG_RESULT stgAccountingImpl(const char* userName, const char* password, const STG_PAIR* pairs)
{
    return stgRequest(STG_CLIENT::ACCOUNT, userName, password, pairs);
}
