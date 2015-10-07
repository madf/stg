/*
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

/*
 *  FreeRADIUS module for data access via Stargazer
 *
 *  $Revision: 1.8 $
 *  $Date: 2010/08/14 04:15:08 $
 *
 */

#include "iface.h"
#include "stgpair.h"

#include <freeradius/ident.h>
#include <freeradius/radiusd.h>
#include <freeradius/modules.h>

#include <stddef.h> // size_t

typedef struct rlm_stg_t {
    char* address;
} rlm_stg_t;

static const CONF_PARSER module_config[] = {
  { "address",  PW_TYPE_STRING_PTR, offsetof(rlm_stg_t, address), NULL,  "unix:/var/run/stg.sock"},

  { NULL, -1, 0, NULL, NULL }        /* end the list */
};

static void deletePairs(STG_PAIR* pairs)
{
    free(pairs);
}

static size_t toVPS(const STG_PAIR* pairs, VALUE_PAIR** vps)
{
    const STG_PAIR* pair = pairs;
    size_t count = 0;

    while (!emptyPair(pair)) {
        VALUE_PAIR* vp = pairmake(pair->key, pair->value, T_OP_SET);
        pairadd(vps, vp);
        ++pair;
        ++count;
    }

    return count;
}

static size_t toReply(STG_RESULT result, REQUEST* request)
{
    size_t count = 0;

    count += toVPS(result.modify, &request->config_items);
    pairfree(&request->reply->vps);
    count += toVPS(result.reply, &request->reply->vps);

    deletePairs(result.modify);
    deletePairs(result.reply);

    return count;
}

static int countVPS(const VALUE_PAIR* pairs)
{
    unsigned count = 0;
    while (pairs != NULL) {
        ++count;
        pairs = pairs->next;
    }
    return count;
}

static STG_PAIR* fromVPS(const VALUE_PAIR* pairs)
{
    unsigned size = countVPS(pairs);
    STG_PAIR* res = (STG_PAIR*)malloc(sizeof(STG_PAIR) * (size + 1));
    size_t pos = 0;
    while (pairs != NULL) {
        bzero(res[pos].key, sizeof(res[0].key));
        bzero(res[pos].value, sizeof(res[0].value));
        strncpy(res[pos].key, pairs->name, sizeof(res[0].key));
        vp_prints_value(res[pos].value, sizeof(res[0].value), (VALUE_PAIR*)pairs, 0);
        ++pos;
        pairs = pairs->next;
    }
    bzero(res[pos].key, sizeof(res[0].key));
    bzero(res[pos].value, sizeof(res[0].value));
    return res;
}

static int toRLMCode(int code)
{
    switch (code)
    {
        case STG_REJECT:   return RLM_MODULE_REJECT;
        case STG_FAIL:     return RLM_MODULE_FAIL;
        case STG_OK:       return RLM_MODULE_OK;
        case STG_HANDLED:  return RLM_MODULE_HANDLED;
        case STG_INVALID:  return RLM_MODULE_INVALID;
        case STG_USERLOCK: return RLM_MODULE_USERLOCK;
        case STG_NOTFOUND: return RLM_MODULE_NOTFOUND;
        case STG_NOOP:     return RLM_MODULE_NOOP;
        case STG_UPDATED:  return RLM_MODULE_UPDATED;
    }
    return RLM_MODULE_REJECT;
}

/*
 *    Do any per-module initialization that is separate to each
 *    configured instance of the module.  e.g. set up connections
 *    to external databases, read configuration files, set up
 *    dictionary entries, etc.
 *
 *    If configuration information is given in the config section
 *    that must be referenced in later calls, store a handle to it
 *    in *instance otherwise put a null pointer there.
 */
static int stg_instantiate(CONF_SECTION* conf, void** instance)
{
    rlm_stg_t* data;

    /*
     *    Set up a storage area for instance data
     */
    data = rad_malloc(sizeof(*data));
    if (!data)
        return -1;

    memset(data, 0, sizeof(*data));

    /*
     *    If the configuration parameters can't be parsed, then
     *    fail.
     */
    if (cf_section_parse(conf, data, module_config) < 0) {
        free(data);
        return -1;
    }

    if (!stgInstantiateImpl(data->address)) {
        free(data);
        return -1;
    }

    *instance = data;

    return 0;
}

/*
 *    Find the named user in this modules database.  Create the set
 *    of attribute-value pairs to check and reply with for this user
 *    from the database. The authentication code only needs to check
 *    the password, the rest is done here.
 */
static int stg_authorize(void* instance, REQUEST* request)
{
    STG_RESULT result;
    STG_PAIR* pairs = fromVPS(request->packet->vps);
    size_t count = 0;
    const char* username = NULL;
    const char* password = NULL;

    instance = instance;

    DEBUG("rlm_stg: stg_authorize()");

    if (request->username) {
        username = request->username->data.strvalue;
        DEBUG("rlm_stg: stg_authorize() request username field: '%s'", username);
    }

    if (request->password) {
        password = request->password->data.strvalue;
        DEBUG("rlm_stg: stg_authorize() request password field: '%s'", password);
    }

    result = stgAuthorizeImpl(username, password, pairs);
    deletePairs(pairs);

    if (!result.modify && !result.reply) {
        DEBUG("rlm_stg: stg_authorize() failed.");
        return RLM_MODULE_REJECT;
    }

    count = toReply(result, request);

    if (count)
        return RLM_MODULE_UPDATED;

    return toRLMCode(result.returnCode);
}

/*
 *    Authenticate the user with the given password.
 */
static int stg_authenticate(void* instance, REQUEST* request)
{
    STG_RESULT result;
    STG_PAIR* pairs = fromVPS(request->packet->vps);
    size_t count = 0;
    const char* username = NULL;
    const char* password = NULL;

    instance = instance;

    DEBUG("rlm_stg: stg_authenticate()");

    if (request->username) {
        username = request->username->data.strvalue;
        DEBUG("rlm_stg: stg_authenticate() request username field: '%s'", username);
    }

    if (request->password) {
        password = request->password->data.strvalue;
        DEBUG("rlm_stg: stg_authenticate() request password field: '%s'", password);
    }

    result = stgAuthenticateImpl(username, password, pairs);
    deletePairs(pairs);

    if (!result.modify && !result.reply) {
        DEBUG("rlm_stg: stg_authenticate() failed.");
        return RLM_MODULE_REJECT;
    }

    count = toReply(result, request);

    if (count)
        return RLM_MODULE_UPDATED;

    return toRLMCode(result.returnCode);
}

/*
 *    Massage the request before recording it or proxying it
 */
static int stg_preacct(void* instance, REQUEST* request)
{
    STG_RESULT result;
    STG_PAIR* pairs = fromVPS(request->packet->vps);
    size_t count = 0;
    const char* username = NULL;
    const char* password = NULL;

    DEBUG("rlm_stg: stg_preacct()");

    instance = instance;

    if (request->username) {
        username = request->username->data.strvalue;
        DEBUG("rlm_stg: stg_preacct() request username field: '%s'", username);
    }

    if (request->password) {
        password = request->password->data.strvalue;
        DEBUG("rlm_stg: stg_preacct() request password field: '%s'", password);
    }

    result = stgPreAcctImpl(username, password, pairs);
    deletePairs(pairs);

    if (!result.modify && !result.reply) {
        DEBUG("rlm_stg: stg_preacct() failed.");
        return RLM_MODULE_REJECT;
    }

    count = toReply(result, request);

    if (count)
        return RLM_MODULE_UPDATED;

    return toRLMCode(result.returnCode);
}

/*
 *    Write accounting information to this modules database.
 */
static int stg_accounting(void* instance, REQUEST* request)
{
    STG_RESULT result;
    STG_PAIR* pairs = fromVPS(request->packet->vps);
    size_t count = 0;
    const char* username = NULL;
    const char* password = NULL;

    DEBUG("rlm_stg: stg_accounting()");

    instance = instance;

    if (request->username) {
        username = request->username->data.strvalue;
        DEBUG("rlm_stg: stg_accounting() request username field: '%s'", username);
    }

    if (request->password) {
        password = request->password->data.strvalue;
        DEBUG("rlm_stg: stg_accounting() request password field: '%s'", password);
    }

    result = stgAccountingImpl(username, password, pairs);
    deletePairs(pairs);

    if (!result.modify && !result.reply) {
        DEBUG("rlm_stg: stg_accounting() failed.");
        return RLM_MODULE_REJECT;
    }

    count = toReply(result, request);

    if (count)
        return RLM_MODULE_UPDATED;

    return toRLMCode(result.returnCode);
}

/*
 *    See if a user is already logged in. Sets request->simul_count to the
 *    current session count for this user and sets request->simul_mpp to 2
 *    if it looks like a multilink attempt based on the requested IP
 *    address, otherwise leaves request->simul_mpp alone.
 *
 *    Check twice. If on the first pass the user exceeds his
 *    max. number of logins, do a second pass and validate all
 *    logins by querying the terminal server (using eg. SNMP).
 */
static int stg_checksimul(void* instance, REQUEST* request)
{
    DEBUG("rlm_stg: stg_checksimul()");

    instance = instance;

    request->simul_count = 0;

    return RLM_MODULE_OK;
}

static int stg_postauth(void* instance, REQUEST* request)
{
    STG_RESULT result;
    STG_PAIR* pairs = fromVPS(request->packet->vps);
    size_t count = 0;
    const char* username = NULL;
    const char* password = NULL;

    DEBUG("rlm_stg: stg_postauth()");

    instance = instance;

    if (request->username) {
        username = request->username->data.strvalue;
        DEBUG("rlm_stg: stg_postauth() request username field: '%s'", username);
    }

    if (request->password) {
        password = request->password->data.strvalue;
        DEBUG("rlm_stg: stg_postauth() request password field: '%s'", password);
    }

    result = stgPostAuthImpl(username, password, pairs);
    deletePairs(pairs);

    if (!result.modify && !result.reply) {
        DEBUG("rlm_stg: stg_postauth() failed.");
        return RLM_MODULE_REJECT;
    }

    count = toReply(result, request);

    if (count)
        return RLM_MODULE_UPDATED;

    return toRLMCode(result.returnCode);
}

static int stg_detach(void* instance)
{
    free(((struct rlm_stg_t*)instance)->address);
    free(instance);
    return 0;
}

module_t rlm_stg = {
    RLM_MODULE_INIT,
    "stg",
    RLM_TYPE_THREAD_UNSAFE, /* type */
    stg_instantiate,      /* instantiation */
    stg_detach,           /* detach */
    {
        stg_authenticate, /* authentication */
        stg_authorize,    /* authorization */
        stg_preacct,      /* preaccounting */
        stg_accounting,   /* accounting */
        stg_checksimul,   /* checksimul */
        NULL,    /* pre-proxy */
        NULL,   /* post-proxy */
        stg_postauth      /* post-auth */
    },
};
