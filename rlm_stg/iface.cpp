#include "iface.h"

#include "stg_client.h"
#include "types.h"
#include "radlog.h"

#include <stdexcept>
#include <cstring>

#include <strings.h>

namespace RLM = STG::RLM;

using RLM::Client;
using RLM::PAIRS;
using RLM::RESULT;
using RLM::REQUEST_TYPE;

namespace
{

STG_PAIR* toSTGPairs(const PAIRS& source)
{
    STG_PAIR * pairs = new STG_PAIR[source.size() + 1];
    for (size_t pos = 0; pos < source.size(); ++pos) {
        bzero(pairs[pos].key, sizeof(pairs[pos].key));
        bzero(pairs[pos].value, sizeof(pairs[pos].value));
        strncpy(pairs[pos].key, source[pos].first.c_str(), sizeof(pairs[pos].key));
        strncpy(pairs[pos].value, source[pos].second.c_str(), sizeof(pairs[pos].value));
    }
    bzero(pairs[source.size()].key, sizeof(pairs[source.size()].key));
    bzero(pairs[source.size()].value, sizeof(pairs[source.size()].value));

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
    result.returnCode = source.returnCode;
    return result;
}

STG_RESULT emptyResult()
{
    STG_RESULT result = {NULL, NULL, STG_REJECT};
    return result;
}

std::string toString(const char* value)
{
    if (value == NULL)
        return "";
    else
        return value;
}

STG_RESULT stgRequest(REQUEST_TYPE type, const char* userName, const char* password, const STG_PAIR* pairs)
{
    Client* client = Client::get();
    if (client == NULL) {
        RadLog("Client is not configured.");
        return emptyResult();
    }
    try {
        return toResult(client->request(type, toString(userName), toString(password), fromSTGPairs(pairs)));
    } catch (const std::runtime_error& ex) {
        RadLog("Error: '%s'.", ex.what());
        return emptyResult();
    }
}

}

int stgInstantiateImpl(const char* address)
{
    if (Client::configure(toString(address)))
        return 1;

    return 0;
}

STG_RESULT stgAuthorizeImpl(const char* userName, const char* password, const STG_PAIR* pairs)
{
    return stgRequest(RLM::AUTHORIZE, userName, password, pairs);
}

STG_RESULT stgAuthenticateImpl(const char* userName, const char* password, const STG_PAIR* pairs)
{
    return stgRequest(RLM::AUTHENTICATE, userName, password, pairs);
}

STG_RESULT stgPostAuthImpl(const char* userName, const char* password, const STG_PAIR* pairs)
{
    return stgRequest(RLM::POST_AUTH, userName, password, pairs);
}

STG_RESULT stgPreAcctImpl(const char* userName, const char* password, const STG_PAIR* pairs)
{
    return stgRequest(RLM::PRE_ACCT, userName, password, pairs);
}

STG_RESULT stgAccountingImpl(const char* userName, const char* password, const STG_PAIR* pairs)
{
    return stgRequest(RLM::ACCOUNT, userName, password, pairs);
}
