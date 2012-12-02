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

#ifndef NDEBUG
#define NDEBUG
#include <freeradius/ident.h>
#include <freeradius/radiusd.h>
#include <freeradius/modules.h>
#undef NDEBUG
#endif

#include "stgpair.h"
#include "iface.h"

typedef struct rlm_stg_t {
    char * server;
    uint16_t port;
    char * password;
} rlm_stg_t;

static const CONF_PARSER module_config[] = {
  { "server",  PW_TYPE_STRING_PTR, offsetof(rlm_stg_t,server), NULL,  "localhost"},
  { "port",  PW_TYPE_INTEGER,     offsetof(rlm_stg_t,port), NULL,  "9091" },
  { "password",  PW_TYPE_STRING_PTR, offsetof(rlm_stg_t,password), NULL,  "123456"},

  { NULL, -1, 0, NULL, NULL }        /* end the list */
};

int emptyPair(const STG_PAIR * pair);

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
static int stg_instantiate(CONF_SECTION *conf, void **instance)
{
    rlm_stg_t *data;

    /*
     *    Set up a storage area for instance data
     */
    data = rad_malloc(sizeof(*data));
    if (!data) {
        return -1;
    }
    memset(data, 0, sizeof(*data));

    /*
     *    If the configuration parameters can't be parsed, then
     *    fail.
     */
    if (cf_section_parse(conf, data, module_config) < 0) {
        free(data);
        return -1;
    }

    if (!stgInstantiateImpl(data->server, data->port)) {
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
static int stg_authorize(void *, REQUEST *request)
{
    VALUE_PAIR * pwd;
    VALUE_PAIR * svc;
    const STG_PAIR * pairs;
    const STG_PAIR * pair;
    size_t count = 0;

    instance = instance;

    DEBUG("rlm_stg: stg_authorize()");

    if (request->username) {
        DEBUG("rlm_stg: stg_authorize() request username field: '%s'", request->username->vp_strvalue);
    }
    if (request->password) {
        DEBUG("rlm_stg: stg_authorize() request password field: '%s'", request->password->vp_strvalue);
    }
    // Here we need to define Framed-Protocol
    svc = pairfind(request->packet->vps, PW_SERVICE_TYPE);
    if (svc) {
        DEBUG("rlm_stg: stg_authorize() Service-Type defined as '%s'", svc->vp_strvalue);
        pairs = stgAuthorizeImpl((const char *)request->username->vp_strvalue, (const char *)svc->vp_strvalue);
    } else {
        DEBUG("rlm_stg: stg_authorize() Service-Type undefined");
        pairs = stgAuthorizeImpl((const char *)request->username->vp_strvalue, "");
    }
    if (!pairs) {
        DEBUG("rlm_stg: stg_authorize() failed.");
        return RLM_MODULE_REJECT;
    }

    pair = pairs;
    while (!emptyPair(pair)) {
        pwd = pairmake(pair->key, pair->value, T_OP_SET);
        pairadd(&request->config_items, pwd);
        DEBUG("Adding pair '%s': '%s'", pair->key, pair->value);
        ++pair;
        ++count;
    }
    deletePairs(pairs);

    if (count)
        return RLM_MODULE_UPDATED;

    return RLM_MODULE_NOOP;
}

/*
 *    Authenticate the user with the given password.
 */
static int stg_authenticate(void *, REQUEST *request)
{
    VALUE_PAIR * svc;
    VALUE_PAIR * pwd;
    const STG_PAIR * pairs;
    const STG_PAIR * pair;
    size_t count = 0;

    instance = instance;

    DEBUG("rlm_stg: stg_authenticate()");

    svc = pairfind(request->packet->vps, PW_SERVICE_TYPE);
    if (svc) {
        DEBUG("rlm_stg: stg_authenticate() Service-Type defined as '%s'", svc->vp_strvalue);
        pairs = stgAuthenticateImpl((const char *)request->username->vp_strvalue, (const char *)svc->vp_strvalue);
    } else {
        DEBUG("rlm_stg: stg_authenticate() Service-Type undefined");
        pairs = stgAuthenticateImpl((const char *)request->username->vp_strvalue, "");
    }
    if (!pairs) {
        DEBUG("rlm_stg: stg_authenticate() failed.");
        return RLM_MODULE_REJECT;
    }

    pair = pairs;
    while (!emptyPair(pair)) {
        pwd = pairmake(pair->key, pair->value, T_OP_SET);
        pairadd(&request->reply->vps, pwd);
        ++pair;
        ++count;
    }
    deletePairs(pairs);

    if (count)
        return RLM_MODULE_UPDATED;

    return RLM_MODULE_NOOP;
}

/*
 *    Massage the request before recording it or proxying it
 */
static int stg_preacct(void *, REQUEST *)
{
    DEBUG("rlm_stg: stg_preacct()");

    instance = instance;

    return RLM_MODULE_OK;
}

/*
 *    Write accounting information to this modules database.
 */
static int stg_accounting(void *, REQUEST * request)
{
    VALUE_PAIR * sttype;
    VALUE_PAIR * svc;
    VALUE_PAIR * sessid;
    VALUE_PAIR * pwd;
    const STG_PAIR * pairs;
    const STG_PAIR * pair;
    size_t count = 0;

    instance = instance;

    DEBUG("rlm_stg: stg_accounting()");

    svc = pairfind(request->packet->vps, PW_SERVICE_TYPE);
    sessid = pairfind(request->packet->vps, PW_ACCT_SESSION_ID);
    sttype = pairfind(request->packet->vps, PW_ACCT_STATUS_TYPE);

    if (!sessid) {
        DEBUG("rlm_stg: stg_accounting() Acct-Session-ID undefined");
        return RLM_MODULE_FAIL;
    }

    if (sttype) {
        DEBUG("Acct-Status-Type := %s", sttype->vp_strvalue);
        if (svc) {
            DEBUG("rlm_stg: stg_accounting() Service-Type defined as '%s'", svc->vp_strvalue);
            pairs = stgAccountingImpl((const char *)request->username->vp_strvalue, (const char *)svc->vp_strvalue, (const char *)sttype->vp_strvalue, (const char *)sessid->vp_strvalue);
        } else {
            DEBUG("rlm_stg: stg_accounting() Service-Type undefined");
            pairs = stgAccountingImpl((const char *)request->username->vp_strvalue, (const char *)svc->vp_strvalue, (const char *)sttype->vp_strvalue, (const char *)sessid->vp_strvalue);
        }
    } else {
        DEBUG("rlm_stg: stg_accounting() Acct-Status-Type := NULL");
        return RLM_MODULE_OK;
    }
    if (!pairs) {
        DEBUG("rlm_stg: stg_accounting() failed.");
        return RLM_MODULE_REJECT;
    }

    pair = pairs;
    while (!emptyPair(pair)) {
        pwd = pairmake(pair->key, pair->value, T_OP_SET);
        pairadd(&request->reply->vps, pwd);
        ++pair;
        ++count;
    }
    deletePairs(pairs);

    if (count)
        return RLM_MODULE_UPDATED;

    return RLM_MODULE_OK;
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
static int stg_checksimul(void *, REQUEST *request)
{
    DEBUG("rlm_stg: stg_checksimul()");

    instance = instance;

    request->simul_count=0;

    return RLM_MODULE_OK;
}

static int stg_postauth(void *, REQUEST *request)
{
    VALUE_PAIR * svc;
    VALUE_PAIR * pwd;
    const STG_PAIR * pairs;
    const STG_PAIR * pair;
    size_t count = 0;

    instance = instance;

    DEBUG("rlm_stg: stg_postauth()");

    svc = pairfind(request->packet->vps, PW_SERVICE_TYPE);

    if (svc) {
        DEBUG("rlm_stg: stg_postauth() Service-Type defined as '%s'", svc->vp_strvalue);
        pairs = stgPostAuthImpl((const char *)request->username->vp_strvalue, (const char *)svc->vp_strvalue);
    } else {
        DEBUG("rlm_stg: stg_postauth() Service-Type undefined");
        pairs = stgPostAuthImpl((const char *)request->username->vp_strvalue, "");
    }
    if (!pairs) {
        DEBUG("rlm_stg: stg_postauth() failed.");
        return RLM_MODULE_REJECT;
    }

    pair = pairs;
    while (!emptyPair(pair)) {
        pwd = pairmake(pair->key, pair->value, T_OP_SET);
        pairadd(&request->reply->vps, pwd);
        ++pair;
        ++count;
    }
    deletePairs(pairs);

    if (count)
        return RLM_MODULE_UPDATED;

    return RLM_MODULE_NOOP;
}

static int stg_detach(void *instance)
{
    free(((struct rlm_stg_t *)instance)->server);
    free(instance);
    return 0;
}

/*
 *    The module name should be the only globally exported symbol.
 *    That is, everything else should be 'static'.
 *
 *    If the module needs to temporarily modify it's instantiation
 *    data, the type should be changed to RLM_TYPE_THREAD_UNSAFE.
 *    The server will then take care of ensuring that the module
 *    is single-threaded.
 */
module_t rlm_stg = {
    RLM_MODULE_INIT,
    "stg",
    RLM_TYPE_THREAD_SAFE,        /* type */
    stg_instantiate,        /* instantiation */
    stg_detach,            /* detach */
    {
        stg_authenticate,    /* authentication */
        stg_authorize,    /* authorization */
        stg_preacct,    /* preaccounting */
        stg_accounting,    /* accounting */
        stg_checksimul,    /* checksimul */
        NULL,            /* pre-proxy */
        NULL,            /* post-proxy */
        stg_postauth            /* post-auth */
    },
};
