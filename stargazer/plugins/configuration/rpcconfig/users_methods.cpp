#include <cerrno>

#include "stg/users.h"
#include "stg/admins.h"
#include "stg/tariffs.h"
#include "stg/user_ips.h"
#include "stg/common.h"
#include "stg/user_property.h"

#include "users_methods.h"
#include "rpcconfig.h"
#include "user_helper.h"

//------------------------------------------------------------------------------

void METHOD_USER_GET::execute(xmlrpc_c::paramList const & paramList,
                              xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
std::string login = paramList.getString(1);
paramList.verifyEnd(2);

std::map<std::string, xmlrpc_c::value> structVal;
ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    structVal["result"] = xmlrpc_c::value_boolean(false);
    *retvalPtr = xmlrpc_c::value_struct(structVal);
    return;
    }

USER_PTR u;

if (users->FindByName(login, &u))
    {
    structVal["result"] = xmlrpc_c::value_boolean(false);
    *retvalPtr = xmlrpc_c::value_struct(structVal);
    return;
    }

USER_HELPER uhelper(u, *users);

if (!adminInfo.priviledges.userConf || !adminInfo.priviledges.userPasswd)
    {
    uhelper.GetUserInfo(retvalPtr, true);
    return;
    }

uhelper.GetUserInfo(retvalPtr);
}

//------------------------------------------------------------------------------

void METHOD_USER_ADD::execute(xmlrpc_c::paramList const & paramList,
                              xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
std::string login = paramList.getString(1);
paramList.verifyEnd(2);

ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

ADMIN * admin = NULL;

if (admins->Find(adminInfo.admin, &admin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

USER_PTR u;

if (users->FindByName(login, &u))
    {
    if (users->Add(login, admin))
        {
        *retvalPtr = xmlrpc_c::value_boolean(false);
        return;
        }

    *retvalPtr = xmlrpc_c::value_boolean(true);
    return;
    }
    
*retvalPtr = xmlrpc_c::value_boolean(false);
return;
}

//------------------------------------------------------------------------------

void METHOD_USER_DEL::execute(xmlrpc_c::paramList const & paramList,
                              xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
std::string login = paramList.getString(1);
paramList.verifyEnd(2);

ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

ADMIN * admin;

if (admins->Find(adminInfo.admin, &admin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

USER_PTR u;

if (!users->FindByName(login, &u))
    {
    users->Del(login, admin);
    *retvalPtr = xmlrpc_c::value_boolean(true);
    return;
    }

*retvalPtr = xmlrpc_c::value_boolean(false);
return;
}

//------------------------------------------------------------------------------

void METHOD_USERS_GET::execute(xmlrpc_c::paramList const & paramList,
                               xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
paramList.verifyEnd(1);

std::map<std::string, xmlrpc_c::value> structVal;
std::vector<xmlrpc_c::value> retval;
ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    structVal["result"] = xmlrpc_c::value_boolean(false);
    *retvalPtr = xmlrpc_c::value_struct(structVal);
    return;
    }

bool hidePassword = !adminInfo.priviledges.userConf ||
                    !adminInfo.priviledges.userPasswd;

USER_PTR u;

int h = users->OpenSearch();
if (!h)
    {
    printfd(__FILE__, "users->OpenSearch() error\n");
    users->CloseSearch(h);
    return;
    }

while (1)
    {
    if (users->SearchNext(h, &u))
        {
        break;
        }

    xmlrpc_c::value info;

    USER_HELPER uhelper(u, *users);

    uhelper.GetUserInfo(&info, hidePassword);

    retval.push_back(info);
    }

*retvalPtr = xmlrpc_c::value_array(retval);
}

//------------------------------------------------------------------------------

void METHOD_USER_CHG::execute(xmlrpc_c::paramList const & paramList,
                              xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
std::string login = paramList.getString(1);
xmlrpc_c::value_struct info(paramList.getStruct(2));
paramList.verifyEnd(3);

ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

ADMIN * admin;

if (admins->Find(adminInfo.admin, &admin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

USER_PTR u;

if (users->FindByName(login, &u))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

USER_HELPER uhelper(u, *users);

if (!adminInfo.priviledges.userConf || !adminInfo.priviledges.userPasswd)
    {
    uhelper.SetUserInfo(info, admin, login, *store, tariffs);
    }
else
    {
    uhelper.SetUserInfo(info, admin, login, *store, tariffs);
    }

u->WriteConf();
u->WriteStat();

*retvalPtr = xmlrpc_c::value_boolean(true);
}

//------------------------------------------------------------------------------

void METHOD_USER_CASH_ADD::execute(xmlrpc_c::paramList const & paramList,
                                   xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
std::string login = paramList.getString(1);
double amount = paramList.getDouble(2);
std::string comment = IconvString(paramList.getString(3), "UTF-8", "KOI8-R");
paramList.verifyEnd(4);

ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

ADMIN * admin;

if (admins->Find(adminInfo.admin, &admin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

USER_PTR u;

if (users->FindByName(login, &u))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

double cash = u->GetProperty().cash.Get();
cash += amount;

if (!u->GetProperty().cash.Set(cash, admin, login, store, comment))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

u->WriteStat();

*retvalPtr = xmlrpc_c::value_boolean(true);
}

//------------------------------------------------------------------------------

void METHOD_USER_CASH_SET::execute(xmlrpc_c::paramList const & paramList,
                                   xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
std::string login = paramList.getString(1);
double cash = paramList.getDouble(2);
std::string comment = IconvString(paramList.getString(3), "UTF-8", "KOI8-R");
paramList.verifyEnd(4);

ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

ADMIN * admin;

if (admins->Find(adminInfo.admin, &admin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

USER_PTR u;

if (users->FindByName(login, &u))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

if (!u->GetProperty().cash.Set(cash, admin, login, store, comment))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

u->WriteStat();

*retvalPtr = xmlrpc_c::value_boolean(true);
}

//------------------------------------------------------------------------------

void METHOD_USER_TARIFF_CHANGE::execute(xmlrpc_c::paramList const & paramList,
                                        xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
std::string login = paramList.getString(1);
std::string tariff = paramList.getString(2);
bool delayed = paramList.getBoolean(3);
std::string comment = IconvString(paramList.getString(4), "UTF-8", "KOI8-R");
paramList.verifyEnd(5);

ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

ADMIN * admin;

if (admins->Find(adminInfo.admin, &admin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

USER_PTR u;

if (users->FindByName(login, &u))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

if (tariffs->FindByName(tariff))
    {
    if (delayed)
        {
        if (u->GetProperty().nextTariff.Set(tariff,
                                            admin,
                                            login,
                                            store,
                                            comment))
            {
            u->WriteConf();
            *retvalPtr = xmlrpc_c::value_boolean(true);
            return;
            }
        }
    else
        {
        const TARIFF * newTariff = tariffs->FindByName(tariff);
        if (newTariff)
            {
            const TARIFF * currentTariff = u->GetTariff();
            std::string message = currentTariff->TariffChangeIsAllowed(*newTariff, stgTime);
            if (message.empty())
                {
                if (u->GetProperty().tariffName.Set(tariff,
                                            admin,
                                            login,
                                            store,
                                            comment))
                    {
                    u->ResetNextTariff();
                    u->WriteConf();
                    *retvalPtr = xmlrpc_c::value_boolean(true);
                    return;
                    }
                }
            else
                {
                GetStgLogger()("Tariff change is prohibited for user %s. %s", u->GetLogin().c_str(), message.c_str());
                }
            }
        }
    }

*retvalPtr = xmlrpc_c::value_boolean(false);
}

//------------------------------------------------------------------------------

void METHOD_GET_ONLINE_IPS::execute(xmlrpc_c::paramList const & paramList,
                                    xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
typedef std::vector<xmlrpc_c::value> ValueVector;
ValueVector subnetsStr = paramList.getArray(1);
paramList.verifyEnd(2);

std::vector<IP_MASK> subnets;

for (ValueVector::const_iterator it(subnetsStr.begin()); it != subnetsStr.end(); ++it)
    {
    IP_MASK ipm;
    if (ParseNet(xmlrpc_c::value_string(*it), ipm))
        {
        printfd(__FILE__, "METHOD_GET_ONLINE_IPS::execute(): Failed to parse subnet ('%s')\n", std::string(xmlrpc_c::value_string(*it)).c_str());
        }
    else
        {
        subnets.push_back(ipm);
        }
    }

std::map<std::string, xmlrpc_c::value> structVal;
ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    structVal["result"] = xmlrpc_c::value_boolean(false);
    *retvalPtr = xmlrpc_c::value_struct(structVal);
    return;
    }

ValueVector ips;

USER_PTR u;

int handle = users->OpenSearch();
if (!handle)
    {
    printfd(__FILE__, "users->OpenSearch() error\n");
    users->CloseSearch(handle);
    return;
    }

while (1)
    {
    if (users->SearchNext(handle, &u))
        {
        break;
        }

    if (u->GetAuthorized())
        {
        uint32_t ip = u->GetCurrIP();

        for (std::vector<IP_MASK>::const_iterator it(subnets.begin()); it != subnets.end(); ++it)
            {
            if ((it->ip & it->mask) == (ip & it->mask))
                {
                ips.push_back(xmlrpc_c::value_string(inet_ntostring(u->GetCurrIP())));
                break;
                }
            }
        }
    }

structVal["ips"] = xmlrpc_c::value_array(ips);

*retvalPtr = xmlrpc_c::value_struct(structVal);
}

bool METHOD_GET_ONLINE_IPS::ParseNet(const std::string & net, IP_MASK & ipm) const
{
size_t pos = net.find_first_of('/');

if (pos == std::string::npos)
    {
    printfd(__FILE__, "METHOD_GET_ONLINE_IPS::ParseNet(): Network address is not in CIDR-notation\n");
    return true;
    }

int res = inet_pton(AF_INET, net.substr(0, pos).c_str(), &ipm.ip);

if (res < 0)
    {
    printfd(__FILE__, "METHOD_GET_ONLINE_IPS::ParseNet(): '%s'\n", strerror(errno));
    return true;
    }
else if (res == 0)
    {
    printfd(__FILE__, "METHOD_GET_ONLINE_IPS::ParseNet(): Invalid network address\n", strerror(errno));
    return true;
    }

if (str2x(net.substr(pos + 1, net.length() - pos - 1), ipm.mask))
    {
    printfd(__FILE__, "METHOD_GET_ONLINE_IPS::ParseNet(): Invalid network mask\n");
    return true;
    }
if (ipm.mask > 32)
    {
    printfd(__FILE__, "METHOD_GET_ONLINE_IPS::ParseNet(): Network mask is out of range\n");
    return true;
    }
ipm.mask = htonl(0xffFFffFF << (32 - ipm.mask));

return false;
}

void METHOD_GET_USER_AUTH_BY::execute(xmlrpc_c::paramList const & paramList,
                                      xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
std::string login = paramList.getString(1);
paramList.verifyEnd(2);

std::map<std::string, xmlrpc_c::value> structVal;
ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    structVal["result"] = xmlrpc_c::value_boolean(false);
    *retvalPtr = xmlrpc_c::value_struct(structVal);
    return;
    }

USER_PTR u;

if (users->FindByName(login, &u))
    {
    structVal["result"] = xmlrpc_c::value_boolean(false);
    *retvalPtr = xmlrpc_c::value_struct(structVal);
    return;
    }

std::vector<std::string> list(u->GetAuthorizers());
std::vector<xmlrpc_c::value> authList;
for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it)
    authList.push_back(xmlrpc_c::value_string(*it));
*retvalPtr = xmlrpc_c::value_array(authList);
}
