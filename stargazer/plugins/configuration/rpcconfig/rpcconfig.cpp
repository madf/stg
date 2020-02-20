
#include "rpcconfig.h"
#include "info_methods.h"
#include "users_methods.h"
#include "tariffs_methods.h"
#include "admins_methods.h"
#include "messages_methods.h"

#include "stg/common.h"
#include "stg/admin.h"
#include "stg/module_settings.h"
#include "stg/settings.h"

#include <algorithm>
#include <vector>
#include <ostream> // xmlrpc-c devs have missed something :)
#include <cstdlib>
#include <csignal>
#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

RPC_CONFIG_SETTINGS::RPC_CONFIG_SETTINGS()
    : port(0),
      cookieTimeout(0)
{
}

int RPC_CONFIG_SETTINGS::ParseSettings(const MODULE_SETTINGS & s)
{
PARAM_VALUE pv;
pv.param = "Port";
std::vector<PARAM_VALUE>::const_iterator pvi;
pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
    errorStr = "Parameter \'Port\' not found.";
    printfd(__FILE__, "Parameter 'Port' not found\n");
    return -1;
    }
int p;
if (ParseIntInRange(pvi->value[0], 2, 65535, &p))
    {
    errorStr = "Cannot parse parameter \'Port\': " + errorStr;
    printfd(__FILE__, "Cannot parse parameter 'Port'\n");
    return -1;
    }
port = static_cast<uint16_t>(p);

pv.param = "CookieTimeout";
pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
    cookieTimeout = 1800; // 30 * 60
    }
else
    {
    if (str2x(pvi->value[0], cookieTimeout))
        {
        errorStr = "Incorrect value of CookieTimeout: \'" + pvi->value[0] + "\'";
        printfd(__FILE__, "Incorrect value of 'CookieTimeout'\n");
        return -1;
        }
    }

return 0;
}

extern "C" Plugin* GetPlugin()
{
    static RPC_CONFIG plugin;
    return &plugin;
}

RPC_CONFIG::RPC_CONFIG()
    : users(NULL),
      admins(NULL),
      tariffs(NULL),
      store(NULL),
      fd(-1),
      rpcServer(NULL),
      running(false),
      stopped(true),
      dayFee(0),
      logger(PluginLogger::get("conf_rpc"))
{
}

RPC_CONFIG::~RPC_CONFIG()
{
// delete server
delete rpcServer;
}

int RPC_CONFIG::ParseSettings()
{
int ret = rpcConfigSettings.ParseSettings(settings);

if (ret)
    errorStr = rpcConfigSettings.GetStrError();

return ret;
}

void RPC_CONFIG::SetStgSettings(const SETTINGS * s)
{
    dayFee = s->GetDayFee();
    dirNames.erase(dirNames.begin(), dirNames.end());
    for (size_t i = 0; i < DIR_NUM; ++i) {
        dirNames.push_back(s->GetDirName(i));
    }
}

int RPC_CONFIG::Start()
{
InitiateRegistry();
running = true;

fd = socket(AF_INET, SOCK_STREAM, 0);
if (fd < 0)
    {
    errorStr = "Failed to create socket";
    logger("Cannot create a socket: %s", strerror(errno));
    printfd(__FILE__, "Failed to create listening socket: %s\n", strerror(errno));
    return -1;
    }

int flag = 1;

if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)))
    {
    errorStr = "Setsockopt failed.";
    logger("setsockopt error: %s", strerror(errno));
    printfd(__FILE__, "Setsockopt failed: %s\n", strerror(errno));
    return -1;
    }

struct sockaddr_in addr;
addr.sin_family = AF_INET;
addr.sin_port = htons(rpcConfigSettings.GetPort());
addr.sin_addr.s_addr = inet_addr("0.0.0.0");

if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)))
    {
    logger("Cannot bind the socket: %s", strerror(errno));
    errorStr = "Failed to bind socket";
    printfd(__FILE__, "Failed to bind listening socket: %s\n", strerror(errno));
    return -1;
    }

if (listen(fd, 10))
    {
    logger("Cannot listen the socket: %s", strerror(errno));
    errorStr = "Failed to listen socket";
    printfd(__FILE__, "Failed to listen listening socket: %s\n", strerror(errno));
    return -1;
    }

rpcServer = new xmlrpc_c::serverAbyss(
        xmlrpc_c::serverAbyss::constrOpt()
        .registryP(&rpcRegistry)
        .logFileName("/var/log/stargazer_rpc.log")
        .socketFd(fd)
        );

if (pthread_create(&tid, NULL, Run, this))
    {
    errorStr = "Failed to create RPC thread";
    logger("Cannot create RPC thread.");
    printfd(__FILE__, "Failed to crate RPC thread\n");
    return -1;
    }

return 0;
}

int RPC_CONFIG::Stop()
{
running = false;
for (int i = 0; i < 5 && !stopped; ++i)
    {
    struct timespec ts = {0, 200000000};
    nanosleep(&ts, NULL);
    }

if (!stopped)
    {
    running = true;
    logger("Cannot stop RPC thread.");
    printfd(__FILE__, "Failed to stop RPC thread\n");
    errorStr = "Failed to stop RPC thread";
    return -1;
    }
else
    {
    pthread_join(tid, NULL);
    }

close(fd);

return 0;
}

void * RPC_CONFIG::Run(void * rc)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

RPC_CONFIG * config = static_cast<RPC_CONFIG *>(rc);

config->stopped = false;
while (config->running)
    {
    if (WaitPackets(config->fd))
        config->rpcServer->runOnce();
    }
config->stopped = true;

return NULL;
}

bool RPC_CONFIG::GetAdminInfo(const std::string & cookie,
                              ADMIN_INFO * info)
{
std::map<std::string,
         ADMIN_INFO>::iterator it;

it = cookies.find(cookie);

if (it == cookies.end())
    {
    return true;
    }

if (difftime(it->second.accessTime, time(NULL)) >
    rpcConfigSettings.GetCookieTimeout())
    {
    cookies.erase(it);
    return true;
    }

// Update access time
time(&it->second.accessTime);
*info = it->second;
return false;
}

bool RPC_CONFIG::CheckAdmin(const std::string & login,
                            const std::string & password,
                            std::string * cookie)
{
ADMIN * admin = NULL;

if (!admins->Correct(login, password, &admin))
    {
    logger("Attempt to connect with invalid credentials. Login: %s", login.c_str());
    return true;
    }

ADMIN_INFO info;
time(&info.accessTime);
info.admin = login;
info.priviledges = *admin->GetPriv();
*cookie = GetCookie();
cookies[*cookie] = info;

return false;
}

bool RPC_CONFIG::LogoutAdmin(const std::string & cookie)
{
std::map<std::string,
         ADMIN_INFO>::iterator it;

it = cookies.find(cookie);

if (it == cookies.end())
    {
    return true;
    }

cookies.erase(it);

return false;
}

std::string RPC_CONFIG::GetCookie() const
{
std::string charset("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
std::string cookie;

for (int i = 0; i < 64; ++i)
    {
    cookie += charset[rand() % charset.length()];
    };

return cookie;
}

void RPC_CONFIG::InitiateRegistry()
{
// manage registry
xmlrpc_c::methodPtr const methodInfoPtr(new METHOD_INFO(
            tariffs,
            users,
            dayFee,
            dirNames
            ));
rpcRegistry.addMethod("stargazer.info", methodInfoPtr);

xmlrpc_c::methodPtr const methodLoginPtr(new METHOD_LOGIN(
            this
            ));
rpcRegistry.addMethod("stargazer.login", methodLoginPtr);

xmlrpc_c::methodPtr const methodLogoutPtr(new METHOD_LOGOUT(
            this
            ));
rpcRegistry.addMethod("stargazer.logout", methodLogoutPtr);

xmlrpc_c::methodPtr const methodGetUserPtr(new METHOD_USER_GET(
            this,
            users
            ));
rpcRegistry.addMethod("stargazer.get_user", methodGetUserPtr);

xmlrpc_c::methodPtr const methodAddUserPtr(new METHOD_USER_ADD(
            this,
            admins,
            users
            ));
rpcRegistry.addMethod("stargazer.add_user", methodAddUserPtr);

xmlrpc_c::methodPtr const methodDelUserPtr(new METHOD_USER_DEL(
            this,
            admins,
            users
            ));
rpcRegistry.addMethod("stargazer.del_user", methodDelUserPtr);

xmlrpc_c::methodPtr const methodGetUsersPtr(new METHOD_USERS_GET(
            this,
            users
            ));
rpcRegistry.addMethod("stargazer.get_users", methodGetUsersPtr);

xmlrpc_c::methodPtr const methodChgUserPtr(new METHOD_USER_CHG(
            this,
            admins,
            tariffs,
            store,
            users
            ));
rpcRegistry.addMethod("stargazer.chg_user", methodChgUserPtr);

xmlrpc_c::methodPtr const methodAddCashPtr(new METHOD_USER_CASH_ADD(
            this,
            admins,
            store,
            users
            ));
rpcRegistry.addMethod("stargazer.add_user_cash", methodAddCashPtr);

xmlrpc_c::methodPtr const methodSetCashPtr(new METHOD_USER_CASH_SET(
            this,
            admins,
            store,
            users
            ));
rpcRegistry.addMethod("stargazer.set_user_cash", methodSetCashPtr);

xmlrpc_c::methodPtr const methodTariffChangePtr(new METHOD_USER_TARIFF_CHANGE(
            this,
            admins,
            tariffs,
            store,
            users
            ));
rpcRegistry.addMethod("stargazer.chg_user_tariff", methodTariffChangePtr);

xmlrpc_c::methodPtr const methodGetTariffPtr(new METHOD_TARIFF_GET(
            this,
            tariffs
            ));
rpcRegistry.addMethod("stargazer.get_tariff", methodGetTariffPtr);

xmlrpc_c::methodPtr const methodChgTariffPtr(new METHOD_TARIFF_CHG(
            this,
            admins,
            tariffs
            ));
rpcRegistry.addMethod("stargazer.chg_tariff", methodChgTariffPtr);

xmlrpc_c::methodPtr const methodGetTariffsPtr(new METHOD_TARIFFS_GET(
            this,
            tariffs
            ));
rpcRegistry.addMethod("stargazer.get_tariffs", methodGetTariffsPtr);

xmlrpc_c::methodPtr const methodAddTariffPtr(new METHOD_TARIFF_ADD(
            this,
            admins,
            tariffs
            ));
rpcRegistry.addMethod("stargazer.add_tariff", methodAddTariffPtr);

xmlrpc_c::methodPtr const methodDelTariffPtr(new METHOD_TARIFF_DEL(
            this,
            admins,
            tariffs,
            users
            ));
rpcRegistry.addMethod("stargazer.del_tariff", methodDelTariffPtr);

xmlrpc_c::methodPtr const methodGetAdminPtr(new METHOD_ADMIN_GET(
            this,
            admins
            ));
rpcRegistry.addMethod("stargazer.get_admin", methodGetAdminPtr);

xmlrpc_c::methodPtr const methodAddAdminPtr(new METHOD_ADMIN_ADD(
            this,
            admins
            ));
rpcRegistry.addMethod("stargazer.add_admin", methodAddAdminPtr);

xmlrpc_c::methodPtr const methodDelAdminPtr(new METHOD_ADMIN_DEL(
            this,
            admins
            ));
rpcRegistry.addMethod("stargazer.del_admin", methodDelAdminPtr);

xmlrpc_c::methodPtr const methodChgAdminPtr(new METHOD_ADMIN_CHG(
            this,
            admins
            ));
rpcRegistry.addMethod("stargazer.chg_admin", methodChgAdminPtr);

xmlrpc_c::methodPtr const methodGetAdminsPtr(new METHOD_ADMINS_GET(
            this,
            admins
            ));
rpcRegistry.addMethod("stargazer.get_admins", methodGetAdminsPtr);

xmlrpc_c::methodPtr const methodSendMessagePtr(new METHOD_MESSAGE_SEND(
            this,
            users
            ));
rpcRegistry.addMethod("stargazer.send_user_message", methodSendMessagePtr);

xmlrpc_c::methodPtr const methodGetOnlinIPsPtr(new METHOD_GET_ONLINE_IPS(
            this,
            users
            ));
rpcRegistry.addMethod("stargazer.get_online_ips", methodGetOnlinIPsPtr);

xmlrpc_c::methodPtr const methodGetUserAuthByPtr(new METHOD_GET_USER_AUTH_BY(
            this,
            users
            ));
rpcRegistry.addMethod("stargazer.get_user_auth_by", methodGetUserAuthByPtr);
}

