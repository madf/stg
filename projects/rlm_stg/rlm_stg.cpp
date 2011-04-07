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

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <exception>

extern "C" {
#include "radius.h"
#include "modules.h"
}

#include "stg_client.h"
#include "stg/common.h"

STG_CLIENT * cli;
volatile time_t stgTime;

/*
 *    Define a structure for our module configuration.
 *
 *    These variables do not need to be in a structure, but it's
 *    a lot cleaner to do so, and a pointer to the structure can
 *    be used as the instance handle.
 */
typedef struct rlm_stg_t {
    char * server;
    char * password;
    uint32_t port;
    uint32_t localPort;
} rlm_stg_t;

/*
 *    A mapping of configuration file names to internal variables.
 *
 *    Note that the string is dynamically allocated, so it MUST
 *    be freed.  When the configuration file parse re-reads the string,
 *    it free's the old one, and strdup's the new one, placing the pointer
 *    to the strdup'd string into 'config.string'.  This gets around
 *    buffer over-flows.
 */
static CONF_PARSER module_config[] = {
  { "password",  PW_TYPE_STRING_PTR, offsetof(rlm_stg_t,password), NULL,  NULL},
  { "server",  PW_TYPE_STRING_PTR, offsetof(rlm_stg_t,server), NULL,  NULL},
  { "port",  PW_TYPE_INTEGER,     offsetof(rlm_stg_t,port), NULL,  "5555" },
  { "local_port", PW_TYPE_INTEGER,    offsetof(rlm_stg_t,localPort), NULL,   "0" },

  { NULL, -1, 0, NULL, NULL }        /* end the list */
};

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
    DEBUG("rlm_stg: stg_instantiate()");
    data = (rlm_stg_t *)rad_malloc(sizeof(rlm_stg_t));
    if (!data) {
        return -1;
    }
    memset(data, 0, sizeof(rlm_stg_t));

    /*
     *    If the configuration parameters can't be parsed, then
     *    fail.
     */
    if (cf_section_parse(conf, data, module_config) < 0) {
        free(data);
        return -1;
    }

    try {
        cli = new STG_CLIENT(data->server, data->port, data->localPort, data->password);
    }
    catch (std::exception & ex) {
        DEBUG("rlm_stg: stg_instantiate() error: '%s'", ex.what());
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
    VALUE_PAIR *uname;
    VALUE_PAIR *pwd;
    VALUE_PAIR *svc;
    DEBUG("rlm_stg: stg_authorize()");

    uname = pairfind(request->packet->vps, PW_USER_NAME);
    if (uname) {
        DEBUG("rlm_stg: stg_authorize() user name defined as '%s'", uname->vp_strvalue);
    } else {
        DEBUG("rlm_stg: stg_authorize() user name undefined");
        return RLM_MODULE_FAIL;
    }
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
        if (cli->Authorize((const char *)request->username->vp_strvalue, (const char *)svc->vp_strvalue)) {
            DEBUG("rlm_stg: stg_authorize() stg status: '%s'", cli->GetError().c_str());
            return RLM_MODULE_REJECT;
        }
    } else {
        DEBUG("rlm_stg: stg_authorize() Service-Type undefined");
        if (cli->Authorize((const char *)request->username->vp_strvalue, "")) {
            DEBUG("rlm_stg: stg_authorize() stg status: '%s'", cli->GetError().c_str());
            return RLM_MODULE_REJECT;
        }
    }
    pwd = pairmake("Cleartext-Password", cli->GetUserPassword().c_str(), T_OP_SET);
    pairadd(&request->config_items, pwd);
    //pairadd(&request->reply->vps, uname);

    return RLM_MODULE_UPDATED;
}

/*
 *    Authenticate the user with the given password.
 */
static int stg_authenticate(void *, REQUEST *request)
{
    /* quiet the compiler */
    VALUE_PAIR *svc;

    DEBUG("rlm_stg: stg_authenticate()");

    svc = pairfind(request->packet->vps, PW_SERVICE_TYPE);
    if (svc) {
        DEBUG("rlm_stg: stg_authenticate() Service-Type defined as '%s'", svc->vp_strvalue);
        if (cli->Authenticate((char *)request->username->vp_strvalue, (const char *)svc->vp_strvalue)) {
            DEBUG("rlm_stg: stg_authenticate() stg status: '%s'", cli->GetError().c_str());
            return RLM_MODULE_REJECT;
        }
    } else {
        DEBUG("rlm_stg: stg_authenticate() Service-Type undefined");
        if (cli->Authenticate((char *)request->username->vp_strvalue, "")) {
            DEBUG("rlm_stg: stg_authenticate() stg status: '%s'", cli->GetError().c_str());
            return RLM_MODULE_REJECT;
        }
    }

    return RLM_MODULE_NOOP;
}

/*
 *    Massage the request before recording it or proxying it
 */
static int stg_preacct(void *, REQUEST *)
{
    DEBUG("rlm_stg: stg_preacct()");

    return RLM_MODULE_OK;
}

/*
 *    Write accounting information to this modules database.
 */
static int stg_accounting(void *, REQUEST * request)
{
    /* quiet the compiler */
    VALUE_PAIR * sttype;
    VALUE_PAIR * svc;
    VALUE_PAIR * sessid;
    svc = pairfind(request->packet->vps, PW_SERVICE_TYPE);

    DEBUG("rlm_stg: stg_accounting()");

    sessid = pairfind(request->packet->vps, PW_ACCT_SESSION_ID);
    if (!sessid) {
        DEBUG("rlm_stg: stg_accounting() Acct-Session-ID undefined");
        return RLM_MODULE_FAIL;
    }
    sttype = pairfind(request->packet->vps, PW_ACCT_STATUS_TYPE);
    if (sttype) {
        DEBUG("Acct-Status-Type := %s", sttype->vp_strvalue);
        if (svc) {
            DEBUG("rlm_stg: stg_accounting() Service-Type defined as '%s'", svc->vp_strvalue);
            if (cli->Account((const char *)sttype->vp_strvalue, (const char *)request->username->vp_strvalue, (const char *)svc->vp_strvalue, (const char *)sessid->vp_strvalue)) {
                DEBUG("rlm_stg: stg_accounting error: '%s'", cli->GetError().c_str());
                return RLM_MODULE_FAIL;
            }
        } else {
            DEBUG("rlm_stg: stg_accounting() Service-Type undefined");
            if (cli->Account((const char *)sttype->vp_strvalue, (const char *)request->username->vp_strvalue, "", (const char *)sessid->vp_strvalue)) {
                DEBUG("rlm_stg: stg_accounting error: '%s'", cli->GetError().c_str());
                return RLM_MODULE_FAIL;
            }
        }
    } else {
        DEBUG("Acct-Status-Type := NULL");
    }

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

    request->simul_count=0;

    return RLM_MODULE_OK;
}

static int stg_postauth(void *, REQUEST *request)
{
    VALUE_PAIR *fia;
    VALUE_PAIR *svc;
    struct in_addr fip;
    DEBUG("rlm_stg: stg_postauth()");
    svc = pairfind(request->packet->vps, PW_SERVICE_TYPE);
    if (svc) {
        DEBUG("rlm_stg: stg_postauth() Service-Type defined as '%s'", svc->vp_strvalue);
        if (cli->PostAuthenticate((const char *)request->username->vp_strvalue, (const char *)svc->vp_strvalue)) {
            DEBUG("rlm_stg: stg_postauth() error: '%s'", cli->GetError().c_str());
            return RLM_MODULE_FAIL;
        }
    } else {
        DEBUG("rlm_stg: stg_postauth() Service-Type undefined");
        if (cli->PostAuthenticate((const char *)request->username->vp_strvalue, "")) {
            DEBUG("rlm_stg: stg_postauth() error: '%s'", cli->GetError().c_str());
            return RLM_MODULE_FAIL;
        }
    }
    if (strncmp((const char *)svc->vp_strvalue, "Framed-User", 11) == 0) {
        fip.s_addr = cli->GetFramedIP();
        DEBUG("rlm_stg: stg_postauth() ip = '%s'", inet_ntostring(fip.s_addr).c_str());
        fia = pairmake("Framed-IP-Address", inet_ntostring(fip.s_addr).c_str(), T_OP_SET);
        pairadd(&request->reply->vps, fia);
    }

    return RLM_MODULE_UPDATED;
}

static int stg_detach(void *instance)
{
    DEBUG("rlm_stg: stg_detach()");
    delete cli;
    free(((struct rlm_stg_t *)instance)->server);
    free(((struct rlm_stg_t *)instance)->password);
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
