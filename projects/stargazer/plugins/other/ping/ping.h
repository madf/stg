 /*
 $Revision: 1.16 $
 $Date: 2009/06/23 11:32:28 $
 $Author: faust $
 */

#ifndef PING_H
#define PING_H

#include <pthread.h>

#include <string>
#include <list>

#include "stg/os_int.h"
#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/notifer.h"
#include "stg/user_ips.h"
#include "stg/pinger.h"
#include "stg/users.h"
#include "stg/logger.h"

extern "C" PLUGIN * GetPlugin();

class PING;
class USER;
class SETTINGS;
//-----------------------------------------------------------------------------*/
class CHG_CURRIP_NOTIFIER_PING: public PROPERTY_NOTIFIER_BASE<uint32_t> {
public:
    CHG_CURRIP_NOTIFIER_PING(const PING & p, USER_PTR u)
        : PROPERTY_NOTIFIER_BASE<uint32_t>(), user(u), ping(p) {}
    CHG_CURRIP_NOTIFIER_PING(const CHG_CURRIP_NOTIFIER_PING & rvalue)
        : PROPERTY_NOTIFIER_BASE<uint32_t>(),
          user(rvalue.user), ping(rvalue.ping) {}
    void Notify(const uint32_t & oldIP, const uint32_t & newIP);
    USER_PTR GetUser() const { return user; }

private:
    CHG_CURRIP_NOTIFIER_PING & operator=(const CHG_CURRIP_NOTIFIER_PING & rvalue);

    USER_PTR user;
    const PING & ping;
};
//-----------------------------------------------------------------------------
class CHG_IPS_NOTIFIER_PING: public PROPERTY_NOTIFIER_BASE<USER_IPS> {
public:
    CHG_IPS_NOTIFIER_PING(const PING & p, USER_PTR u)
        : PROPERTY_NOTIFIER_BASE<USER_IPS>(), user(u), ping(p) {}
    CHG_IPS_NOTIFIER_PING(const CHG_IPS_NOTIFIER_PING & rvalue)
        : PROPERTY_NOTIFIER_BASE<USER_IPS>(),
          user(rvalue.user), ping(rvalue.ping) {}
    void Notify(const USER_IPS & oldIPS, const USER_IPS & newIPS);
    USER_PTR GetUser() const { return user; }

private:
    CHG_IPS_NOTIFIER_PING & operator=(const CHG_IPS_NOTIFIER_PING & rvalue);

    USER_PTR user;
    const PING & ping;
};
//-----------------------------------------------------------------------------
class ADD_USER_NONIFIER_PING: public NOTIFIER_BASE<USER_PTR> {
public:
    ADD_USER_NONIFIER_PING(PING & p) : NOTIFIER_BASE<USER_PTR>(), ping(p) {}
    virtual ~ADD_USER_NONIFIER_PING() {}
    void Notify(const USER_PTR & user);

private:
    ADD_USER_NONIFIER_PING(const ADD_USER_NONIFIER_PING & rvalue);
    ADD_USER_NONIFIER_PING & operator=(const ADD_USER_NONIFIER_PING & rvalue);

    PING & ping;
};
//-----------------------------------------------------------------------------
class DEL_USER_NONIFIER_PING: public NOTIFIER_BASE<USER_PTR> {
public:
    DEL_USER_NONIFIER_PING(PING & p) : NOTIFIER_BASE<USER_PTR>(), ping(p) {}
    virtual ~DEL_USER_NONIFIER_PING() {}
    void Notify(const USER_PTR & user);

private:
    DEL_USER_NONIFIER_PING(const DEL_USER_NONIFIER_PING & rvalue);
    DEL_USER_NONIFIER_PING & operator=(const DEL_USER_NONIFIER_PING & rvalue);

    PING & ping;
};
//-----------------------------------------------------------------------------
class PING_SETTINGS {
public:
    PING_SETTINGS() : pingDelay(0), errorStr() {}
    virtual ~PING_SETTINGS() {}
    const std::string & GetStrError() const { return errorStr; }
    int ParseSettings(const MODULE_SETTINGS & s);
    int GetPingDelay() const { return pingDelay; }
private:
    int pingDelay;
    mutable std::string errorStr;
};
//-----------------------------------------------------------------------------
class PING : public PLUGIN {
friend class CHG_CURRIP_NOTIFIER_PING;
friend class CHG_IPS_NOTIFIER_PING;
public:
    PING();
    virtual ~PING();

    void SetUsers(USERS * u) { users = u; }
    void SetSettings(const MODULE_SETTINGS & s) { settings = s; }
    int ParseSettings();

    int Start();
    int Stop();
    int Reload(const MODULE_SETTINGS & /*ms*/) { return 0; }
    bool IsRunning();

    const std::string & GetStrError() const { return errorStr; }
    std::string GetVersion() const { return "Pinger v.1.01"; }
    uint16_t GetStartPosition() const { return 10; }
    uint16_t GetStopPosition() const { return 10; }

    void AddUser(USER_PTR u);
    void DelUser(USER_PTR u);

private:
    PING(const PING & rvalue);
    PING & operator=(const PING & rvalue);

    void GetUsers();
    void SetUserNotifiers(USER_PTR u);
    void UnSetUserNotifiers(USER_PTR u);
    static void * Run(void * d);

    mutable std::string errorStr;
    PING_SETTINGS pingSettings;
    MODULE_SETTINGS settings;
    USERS * users;
    std::list<USER_PTR> usersList;

    pthread_t thread;
    pthread_mutex_t mutex;
    bool nonstop;
    bool isRunning;
    mutable STG_PINGER pinger;

    std::list<CHG_CURRIP_NOTIFIER_PING> ChgCurrIPNotifierList;
    std::list<CHG_IPS_NOTIFIER_PING> ChgIPNotifierList;

    ADD_USER_NONIFIER_PING onAddUserNotifier;
    DEL_USER_NONIFIER_PING onDelUserNotifier;

    PLUGIN_LOGGER logger;
};
//-----------------------------------------------------------------------------

#endif
