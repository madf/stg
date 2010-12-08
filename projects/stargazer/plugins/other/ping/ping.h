 /*
 $Revision: 1.16 $
 $Date: 2009/06/23 11:32:28 $
 $Author: faust $
 */

#ifndef PING_H
#define PING_H

#include <pthread.h>

#include <string>

#include "os_int.h"
#include "base_plugin.h"
#include "notifer.h"
#include "user_ips.h"
#include "pinger.h"
#include "../../../users.h"

using namespace std;

extern "C" BASE_PLUGIN * GetPlugin();

class PING;
//-----------------------------------------------------------------------------*/
class CHG_CURRIP_NOTIFIER_PING: public PROPERTY_NOTIFIER_BASE<uint32_t>
{
public:
    CHG_CURRIP_NOTIFIER_PING(const PING & p, user_iter u) : user(u), ping(p) {}
    void Notify(const uint32_t & oldIP, const uint32_t & newIP);
    user_iter GetUser() { return user; }

private:
    user_iter user;
    const PING & ping;
};
//-----------------------------------------------------------------------------
class CHG_IPS_NOTIFIER_PING: public PROPERTY_NOTIFIER_BASE<USER_IPS>
{
public:
    CHG_IPS_NOTIFIER_PING(const PING & p, user_iter u) : user(u), ping(p) {}
    void Notify(const USER_IPS & oldIPS, const USER_IPS & newIPS);
    user_iter GetUser() { return user; }

private:
    user_iter user;
    const PING & ping;
};
//-----------------------------------------------------------------------------
class ADD_USER_NONIFIER_PING: public NOTIFIER_BASE<user_iter>
{
public:
    ADD_USER_NONIFIER_PING(PING & p) : ping(p) {}
    virtual ~ADD_USER_NONIFIER_PING() {}
    void Notify(const user_iter & user);

private:
    PING & ping;
};
//-----------------------------------------------------------------------------
class DEL_USER_NONIFIER_PING: public NOTIFIER_BASE<user_iter>
{
public:
    DEL_USER_NONIFIER_PING(PING & p) : ping(p) {}
    virtual ~DEL_USER_NONIFIER_PING() {}
    void Notify(const user_iter & user);

private:
    PING & ping;
};
//-----------------------------------------------------------------------------
class PING_SETTINGS
{
public:
    PING_SETTINGS();
    virtual ~PING_SETTINGS() {}
    const string& GetStrError() const { return errorStr; }
    int ParseSettings(const MODULE_SETTINGS & s);
    int GetPingDelay() { return pingDelay; }
private:
    int ParseIntInRange(const string & str, int min, int max, int * val);

    int pingDelay;
    mutable string errorStr;
};
//-----------------------------------------------------------------------------
class PING: public BASE_PLUGIN
{
friend class CHG_CURRIP_NOTIFIER_PING;
friend class CHG_IPS_NOTIFIER_PING;
public:
    PING();
    virtual ~PING();

    void SetUsers(USERS * u);
    void SetTariffs(TARIFFS *) {}
    void SetAdmins(ADMINS *) {}
    void SetTraffcounter(TRAFFCOUNTER *) {}
    void SetStore(BASE_STORE *) {}
    void SetStgSettings(const SETTINGS *) {}
    void SetSettings(const MODULE_SETTINGS & s);
    int ParseSettings();

    int Start();
    int Stop();
    int Reload() { return 0; }
    bool IsRunning();

    const string & GetStrError() const;
    const string GetVersion() const;
    uint16_t GetStartPosition() const;
    uint16_t GetStopPosition() const;

    void AddUser(user_iter u);
    void DelUser(user_iter u);

private:
    void GetUsers();
    void SetUserNotifiers(user_iter u);
    void UnSetUserNotifiers(user_iter u);
    static void * Run(void * d);

    mutable string errorStr;
    PING_SETTINGS pingSettings;
    MODULE_SETTINGS settings;
    USERS * users;
    list<user_iter> usersList;

    /*
    мы должны перепроверить возможность пингования юзера при изменении
    следующих его параметров:
    - currIP
    - ips
    */
    pthread_t thread;
    pthread_mutex_t mutex;
    bool nonstop;
    bool isRunning;
    mutable STG_PINGER pinger;

    list<CHG_CURRIP_NOTIFIER_PING> ChgCurrIPNotifierList;
    list<CHG_IPS_NOTIFIER_PING> ChgIPNotifierList;

    ADD_USER_NONIFIER_PING onAddUserNotifier;
    DEL_USER_NONIFIER_PING onDelUserNotifier;
};
//-----------------------------------------------------------------------------

#endif
