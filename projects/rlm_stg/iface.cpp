#include "iface.h"

#include "stg_client.h"
#include "radlog.h"

#include <cstring>

#include <strings.h>

namespace
{

struct Response
{
    bool done;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    RESULT result;
    bool status;

    static bool callback(void* data, const RESULT& result, bool status)
    {
        Response& resp = *static_cast<Response*>(data);
        pthread_mutex_lock(&resp.mutex);
        resp.result = result;
        resp.status = status;
        resp.done = true;
        pthread_cond_signal(&resp.cond);
        pthread_mutex_unlock(&resp.mutex);
        return true;
    }
} response;

STG_PAIR* toSTGPairs(const PAIRS& source)
{
    STG_PAIR * pairs = new STG_PAIR[source.size() + 1];
    for (size_t pos = 0; pos < source.size(); ++pos) {
        bzero(pairs[pos].key, sizeof(STG_PAIR::key));
        bzero(pairs[pos].value, sizeof(STG_PAIR::value));
        strncpy(pairs[pos].key, source[pos].first.c_str(), sizeof(STG_PAIR::key));
        strncpy(pairs[pos].value, source[pos].second.c_str(), sizeof(STG_PAIR::value));
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
        RadLog("Client is not configured.");
        return emptyResult();
    }
    try {
        client->request(type, toString(userName), toString(password), fromSTGPairs(pairs));
        pthread_mutex_lock(&response.mutex);
        while (!response.done)
            pthread_cond_wait(&response.cond, &response.mutex);
        pthread_mutex_unlock(&response.mutex);
        if (!response.status)
            return emptyResult();
        return toResult(response.result);
    } catch (const STG_CLIENT::Error& ex) {
        RadLog("Error: '%s'.", ex.what());
        return emptyResult();
    }
}

}

int stgInstantiateImpl(const char* address)
{
    pthread_mutex_init(&response.mutex, NULL);
    pthread_cond_init(&response.cond, NULL);
    response.done = false;

    if (STG_CLIENT::configure(toString(address), &Response::callback, &response))
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
