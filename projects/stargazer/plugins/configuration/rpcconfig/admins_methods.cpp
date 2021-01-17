#include "admins_methods.h"
#include "rpcconfig.h"

#include "stg/common.h"

#include "stg/admins.h"
#include "stg/admin.h"
#include "stg/admin_conf.h"

#include <ostream> // xmlrpc-c devs have missed something :)

//------------------------------------------------------------------------------

void METHOD_ADMIN_GET::execute(xmlrpc_c::paramList const & paramList,
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

STG::Admin * admin;

if (admins->find(login, &admin))
    {
    structVal["result"] = xmlrpc_c::value_boolean(false);
    *retvalPtr = xmlrpc_c::value_struct(structVal);
    return;
    }

structVal["result"] = xmlrpc_c::value_boolean(true);
structVal["login"] = xmlrpc_c::value_string(admin->login());
structVal["password"] = xmlrpc_c::value_string(admin->password());

const auto priv = admin->priv();

structVal["user_stat"] = xmlrpc_c::value_boolean(priv.userStat);
structVal["user_conf"] = xmlrpc_c::value_boolean(priv.userConf);
structVal["user_cash"] = xmlrpc_c::value_boolean(priv.userCash);
structVal["user_passwd"] = xmlrpc_c::value_boolean(priv.userPasswd);
structVal["user_add_del"] = xmlrpc_c::value_boolean(priv.userAddDel);
structVal["admin_chg"] = xmlrpc_c::value_boolean(priv.adminChg);
structVal["tariff_chg"] = xmlrpc_c::value_boolean(priv.tariffChg);

*retvalPtr = xmlrpc_c::value_struct(structVal);
}

//------------------------------------------------------------------------------

void METHOD_ADMIN_ADD::execute(xmlrpc_c::paramList const & paramList,
                               xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
std::string login = paramList.getString(1);
paramList.verifyEnd(2);

ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    printfd(__FILE__, "METHOD_ADMIN_ADD::execute(): 'Not logged or cookie timeout'\n");
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

STG::Admin * admin;

if (admins->find(adminInfo.admin, &admin))
    {
    printfd(__FILE__, "METHOD_ADMIN_ADD::execute(): 'Invalid admin (logged)'\n");
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

if (admins->add(login, *admin))
    {
    printfd(__FILE__, "METHOD_ADMIN_ADD::execute(): 'Failed to add admin'\n");
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

*retvalPtr = xmlrpc_c::value_boolean(true);
}

//------------------------------------------------------------------------------

void METHOD_ADMIN_DEL::execute(xmlrpc_c::paramList const & paramList,
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

STG::Admin * admin;

if (admins->find(adminInfo.admin, &admin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

if (admins->del(login, *admin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

*retvalPtr = xmlrpc_c::value_boolean(true);
}

//------------------------------------------------------------------------------

void METHOD_ADMIN_CHG::execute(xmlrpc_c::paramList const & paramList,
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

STG::Admin * loggedAdmin;

if (admins->find(adminInfo.admin, &loggedAdmin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

STG::Admin * admin;

if (admins->find(login, &admin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

STG::AdminConf conf;

conf.priv = admin->priv();
conf.password = admin->password();
conf.login = login;

std::map<std::string, xmlrpc_c::value> structVal = info;

std::map<std::string, xmlrpc_c::value>::iterator it;

if ((it = structVal.find("password")) != structVal.end())
    {
    conf.password = xmlrpc_c::value_string(it->second);
    }

if ((it = structVal.find("user_stat")) != structVal.end())
    {
    conf.priv.userStat = xmlrpc_c::value_boolean(it->second);
    }

if ((it = structVal.find("user_conf")) != structVal.end())
    {
    conf.priv.userConf = xmlrpc_c::value_boolean(it->second);
    }

if ((it = structVal.find("user_cash")) != structVal.end())
    {
    conf.priv.userCash = xmlrpc_c::value_boolean(it->second);
    }

if ((it = structVal.find("user_passwd")) != structVal.end())
    {
    conf.priv.userPasswd = xmlrpc_c::value_boolean(it->second);
    }

if ((it = structVal.find("user_add_del")) != structVal.end())
    {
    conf.priv.userAddDel = xmlrpc_c::value_boolean(it->second);
    }

if ((it = structVal.find("admin_chg")) != structVal.end())
    {
    conf.priv.adminChg = xmlrpc_c::value_boolean(it->second);
    }

if ((it = structVal.find("tariff_chg")) != structVal.end())
    {
    conf.priv.tariffChg = xmlrpc_c::value_boolean(it->second);
    }

if (admins->change(conf, *loggedAdmin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    }

*retvalPtr = xmlrpc_c::value_boolean(true);
}

//------------------------------------------------------------------------------

void METHOD_ADMINS_GET::execute(xmlrpc_c::paramList const & paramList,
                                xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
paramList.verifyEnd(1);

std::map<std::string, xmlrpc_c::value> mainStructVal;
std::vector<xmlrpc_c::value> retval;
ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    mainStructVal["result"] = xmlrpc_c::value_boolean(false);
    *retvalPtr = xmlrpc_c::value_struct(mainStructVal);
    return;
    }

admins->fmap([&retval](const auto& admin)
    {
        const std::map<std::string, xmlrpc_c::value> structVal{
            {"result", xmlrpc_c::value_boolean(true)},
            {"login", xmlrpc_c::value_string(admin.login())},
            {"password", xmlrpc_c::value_string(admin.password())},
            {"user_stat", xmlrpc_c::value_boolean(admin.priv().userStat)},
            {"user_conf", xmlrpc_c::value_boolean(admin.priv().userConf)},
            {"user_cash", xmlrpc_c::value_boolean(admin.priv().userCash)},
            {"user_passwd", xmlrpc_c::value_boolean(admin.priv().userPasswd)},
            {"user_add_del", xmlrpc_c::value_boolean(admin.priv().userAddDel)},
            {"admin_chg", xmlrpc_c::value_boolean(admin.priv().adminChg)},
            {"tariff_chg", xmlrpc_c::value_boolean(admin.priv().tariffChg)}
        };
        retval.push_back(xmlrpc_c::value_struct(structVal));
    });

*retvalPtr = xmlrpc_c::value_array(retval);
}
