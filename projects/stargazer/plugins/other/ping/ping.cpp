#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "ping.h"
#include "../../../user.h"

class PING_CREATOR
{
private:
    PING * ping;

public:
    PING_CREATOR()
        : ping(new PING())
        {
        };
    ~PING_CREATOR()
        {
        delete ping;
        };

    PING * GetPlugin()
        {
        return ping;
        };
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PING_CREATOR pc;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Класс для поиска юзера в списке нотификаторов
template <typename varType>
class IS_CONTAINS_USER: public binary_function<varType, user_iter, bool>
{
public:
    bool operator()(varType notifier, user_iter user) const
        {
        return notifier.GetUser() == user;
        };
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BASE_PLUGIN * GetPlugin()
{
return pc.GetPlugin();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PING_SETTINGS::PING_SETTINGS()
    : pingDelay(0),
      errorStr()
{
}
//-----------------------------------------------------------------------------
int PING_SETTINGS::ParseSettings(const MODULE_SETTINGS & s)
{
PARAM_VALUE pv;
vector<PARAM_VALUE>::const_iterator pvi;

pv.param = "PingDelay";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end())
    {
    errorStr = "Parameter \'PingDelay\' not found.";
    printfd(__FILE__, "Parameter 'PingDelay' not found\n");
    return -1;
    }
if (ParseIntInRange(pvi->value[0], 5, 3600, &pingDelay))
    {
    errorStr = "Cannot parse parameter \'PingDelay\': " + errorStr;
    printfd(__FILE__, "Canot parse parameter 'PingDelay'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int PING_SETTINGS::ParseIntInRange(const string & str, int min, int max, int * val)
{
if (str2x(str.c_str(), *val))
    {
    errorStr = "Incorrect value \'" + str + "\'.";
    return -1;
    }
if (*val < min || *val > max)
    {
    errorStr = "Value \'" + str + "\' out of range.";
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
PING::PING()
{
pthread_mutex_init(&mutex, NULL);
isRunning = false;
}
//-----------------------------------------------------------------------------
PING::~PING()
{
pthread_mutex_destroy(&mutex);
}
//-----------------------------------------------------------------------------
const string PING::GetVersion() const
{
return "Pinger v.1.01";
}
//-----------------------------------------------------------------------------
void PING::SetSettings(const MODULE_SETTINGS & s)
{
settings = s;
}
//-----------------------------------------------------------------------------
int PING::ParseSettings()
{
int ret = pingSettings.ParseSettings(settings);
if (ret)
    errorStr = pingSettings.GetStrError();
return ret;
}
//-----------------------------------------------------------------------------
void PING::SetUsers(USERS * u)
{
users = u;
}
//-----------------------------------------------------------------------------
const string & PING::GetStrError() const
{
return errorStr;
}
//-----------------------------------------------------------------------------
int PING::Start()
{
GetUsers();

onAddUserNotifier.SetPinger(this);
onDelUserNotifier.SetPinger(this);
users->AddNotifierUserAdd(&onAddUserNotifier);
users->AddNotifierUserDel(&onDelUserNotifier);

nonstop = true;

pinger.SetDelayTime(pingSettings.GetPingDelay());
pinger.Start();

if (pthread_create(&thread, NULL, Run, this))
    {
    errorStr = "Cannot start thread.";
    printfd(__FILE__, "Cannot start thread\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int PING::Stop()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (!isRunning)
    return 0;

pinger.Stop();
nonstop = false;
//5 seconds to thread stops itself
for (int i = 0; i < 25; i++)
    {
    if (!isRunning)
        break;

    usleep(200000);
    }

//after 5 seconds waiting thread still running. now kill it
if (isRunning)
    {
    printfd(__FILE__, "kill PING thread.\n");
    if (pthread_kill(thread, SIGINT))
        {
        errorStr = "Cannot kill PING thread.";
        printfd(__FILE__, "Cannot kill PING thread.\n");
        return -1;
        }
    printfd(__FILE__, "PING killed\n");
    }

users->DelNotifierUserAdd(&onAddUserNotifier);
users->DelNotifierUserDel(&onDelUserNotifier);

list<user_iter>::iterator users_iter;
users_iter = usersList.begin();
while (users_iter != usersList.end())
    {
    UnSetUserNotifiers(*users_iter);
    users_iter++;
    }

return 0;
}
//-----------------------------------------------------------------------------
bool PING::IsRunning()
{
return isRunning;
}
//-----------------------------------------------------------------------------
void * PING::Run(void * d)
{
PING * ping = (PING*)d;
ping->isRunning = true;
list<user_iter>::iterator iter;
uint32_t ip;
time_t t;

while (ping->nonstop)
    {
    iter = ping->usersList.begin();
        {
        STG_LOCKER lock(&ping->mutex, __FILE__, __LINE__);
        while (iter != ping->usersList.end())
            {
            if ((*iter)->property.ips.ConstData().OnlyOneIP())
                {
                ip = (*iter)->property.ips.ConstData()[0].ip;
                if (ping->pinger.GetIPTime(ip, &t) == 0)
                    {
                    if (t)
                        (*iter)->UpdatePingTime(t);
                    }
                }
            else
                {
                ip = (*iter)->GetCurrIP();
                if (ip)
                    {
                    if (ping->pinger.GetIPTime(ip, &t) == 0)
                        {
                        if (t)
                            (*iter)->UpdatePingTime(t);
                        }
                    }
                }
            iter++;
            }
        }
    for (int i = 0; i < 100; i++)
        {
        if (ping->nonstop)
            {
            usleep((10000*ping->pingSettings.GetPingDelay())/3 + 50000);
            }
        }
    }
ping->isRunning = false;
return NULL;
}
//-----------------------------------------------------------------------------
uint16_t PING::GetStartPosition() const
{
return 100;
}
//-----------------------------------------------------------------------------
uint16_t PING::GetStopPosition() const
{
return 100;
}
//-----------------------------------------------------------------------------
void PING::SetUserNotifiers(user_iter u)
{
CHG_CURRIP_NOTIFIER_PING    ChgCurrIPNotifier;
CHG_IPS_NOTIFIER_PING       ChgIPNotifier;

ChgCurrIPNotifier.SetPinger(this);
ChgCurrIPNotifier.SetUser(u);
ChgCurrIPNotifierList.push_front(ChgCurrIPNotifier);

ChgIPNotifier.SetPinger(this);
ChgIPNotifier.SetUser(u);
ChgIPNotifierList.push_front(ChgIPNotifier);

u->AddCurrIPAfterNotifier(&(*ChgCurrIPNotifierList.begin()));
u->property.ips.AddAfterNotifier(&(*ChgIPNotifierList.begin()));
}
//-----------------------------------------------------------------------------
void PING::UnSetUserNotifiers(user_iter u)
{
// ---          CurrIP              ---
IS_CONTAINS_USER<CHG_CURRIP_NOTIFIER_PING>   IsContainsUserCurrIP;
IS_CONTAINS_USER<CHG_IPS_NOTIFIER_PING>      IsContainsUserIP;

list<CHG_CURRIP_NOTIFIER_PING>::iterator     currIPter;
list<CHG_IPS_NOTIFIER_PING>::iterator        IPIter;

currIPter = find_if(ChgCurrIPNotifierList.begin(),
                    ChgCurrIPNotifierList.end(),
                    bind2nd(IsContainsUserCurrIP, u));

if (currIPter != ChgCurrIPNotifierList.end())
    {
    currIPter->GetUser()->DelCurrIPAfterNotifier(&(*currIPter));
    ChgCurrIPNotifierList.erase(currIPter);
    }
// ---         CurrIP end          ---

// ---          IP              ---
IPIter = find_if(ChgIPNotifierList.begin(),
                 ChgIPNotifierList.end(),
                 bind2nd(IsContainsUserIP, u));

if (IPIter != ChgIPNotifierList.end())
    {
    IPIter->GetUser()->property.ips.DelAfterNotifier(&(*IPIter));
    ChgIPNotifierList.erase(IPIter);
    }
// ---          IP end          ---
}
//-----------------------------------------------------------------------------
void PING::GetUsers()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

user_iter u;
int h = users->OpenSearch();
if (!h)
    {
    printfd(__FILE__, "users->OpenSearch() error\n");
    return;
    }

while (users->SearchNext(h, &u) == 0)
    {
    usersList.push_back(u);
    SetUserNotifiers(u);
    if (u->property.ips.ConstData().OnlyOneIP())
        {
        pinger.AddIP(u->property.ips.ConstData()[0].ip);
        }
    else
        {
        uint32_t ip = u->GetCurrIP();
        if (ip)
            {
            pinger.AddIP(ip);
            }
        }
    }

users->CloseSearch(h);
}
//-----------------------------------------------------------------------------
void PING::AddUser(user_iter u)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

SetUserNotifiers(u);
usersList.push_back(u);
}
//-----------------------------------------------------------------------------
void PING::DelUser(user_iter u)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

UnSetUserNotifiers(u);

list<user_iter>::iterator users_iter;
users_iter = usersList.begin();

while (users_iter != usersList.end())
    {
    if (u == *users_iter)
        {
        usersList.erase(users_iter);
        break;
        }
    users_iter++;
    }
}
//-----------------------------------------------------------------------------
void CHG_CURRIP_NOTIFIER_PING::Notify(const uint32_t & oldIP, const uint32_t & newIP)
{
ping->pinger.DelIP(oldIP);
if (newIP)
    {
    ping->pinger.AddIP(newIP);
    }
}
//-----------------------------------------------------------------------------
void CHG_IPS_NOTIFIER_PING::Notify(const USER_IPS & oldIPS, const USER_IPS & newIPS)
{
if (oldIPS.OnlyOneIP())
    {
    ping->pinger.DelIP(oldIPS[0].ip);
    }

if (newIPS.OnlyOneIP())
    {
    ping->pinger.AddIP(newIPS[0].ip);
    }
}
//-----------------------------------------------------------------------------
void ADD_USER_NONIFIER_PING::Notify(const user_iter & user)
{
ping->AddUser(user);
}
//-----------------------------------------------------------------------------
void DEL_USER_NONIFIER_PING::Notify(const user_iter & user)
{
ping->DelUser(user);
}
//-----------------------------------------------------------------------------






