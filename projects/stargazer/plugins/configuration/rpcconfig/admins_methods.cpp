#include "admins_methods.h"

#include "rpcconfig.h"

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

ADMIN admin;

if (admins->FindAdmin(login, &admin))
    {
    structVal["result"] = xmlrpc_c::value_boolean(false);
    *retvalPtr = xmlrpc_c::value_struct(structVal);
    return;
    }

structVal["result"] = xmlrpc_c::value_boolean(true);
structVal["login"] = xmlrpc_c::value_string(admin.GetLogin());
structVal["password"] = xmlrpc_c::value_string(admin.GetPassword());

const PRIV * priv = admin.GetPriv();

structVal["user_stat"] = xmlrpc_c::value_boolean(priv->userStat);
structVal["user_conf"] = xmlrpc_c::value_boolean(priv->userConf);
structVal["user_cash"] = xmlrpc_c::value_boolean(priv->userCash);
structVal["user_passwd"] = xmlrpc_c::value_boolean(priv->userPasswd);
structVal["user_add_del"] = xmlrpc_c::value_boolean(priv->userAddDel);
structVal["admin_chg"] = xmlrpc_c::value_boolean(priv->adminChg);
structVal["tariff_chg"] = xmlrpc_c::value_boolean(priv->tariffChg);

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

ADMIN admin;

if (admins->FindAdmin(adminInfo.admin, &admin))
    {
    printfd(__FILE__, "METHOD_ADMIN_ADD::execute(): 'Invalid admin (logged)'\n");
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

if (admins->Add(login, admin))
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

std::map<std::string, xmlrpc_c::value> structVal;
ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

ADMIN admin;

if (admins->FindAdmin(adminInfo.admin, &admin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

if (admins->Del(login, admin))
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

ADMIN loggedAdmin;

if (admins->FindAdmin(adminInfo.admin, &loggedAdmin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

ADMIN admin;

if (admins->FindAdmin(login, &admin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

ADMIN_CONF conf;

conf.priv = *admin.GetPriv();
conf.password = admin.GetPassword();
conf.login = login;

std::map<std::string, xmlrpc_c::value> structVal(
    static_cast<std::map<std::string, xmlrpc_c::value> >(xmlrpc_c::value_struct(info))
    );

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

if (admins->Change(conf, loggedAdmin))
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

std::map<std::string, xmlrpc_c::value> structVal;
std::vector<xmlrpc_c::value> retval;
ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    structVal["result"] = xmlrpc_c::value_boolean(false);
    *retvalPtr = xmlrpc_c::value_struct(structVal);
    return;
    }

ADMIN_CONF ac;
int h = admins->OpenSearch();

while (admins->SearchNext(h, &ac) == 0)
    {
    std::map<std::string, xmlrpc_c::value> structVal;
    structVal["result"] = xmlrpc_c::value_boolean(true);
    structVal["login"] = xmlrpc_c::value_string(ac.login);
    structVal["password"] = xmlrpc_c::value_string(ac.password);
    structVal["user_stat"] = xmlrpc_c::value_boolean(ac.priv.userStat);
    structVal["user_conf"] = xmlrpc_c::value_boolean(ac.priv.userConf);
    structVal["user_cash"] = xmlrpc_c::value_boolean(ac.priv.userCash);
    structVal["user_passwd"] = xmlrpc_c::value_boolean(ac.priv.userPasswd);
    structVal["user_add_del"] = xmlrpc_c::value_boolean(ac.priv.userAddDel);
    structVal["admin_chg"] = xmlrpc_c::value_boolean(ac.priv.adminChg);
    structVal["tariff_chg"] = xmlrpc_c::value_boolean(ac.priv.tariffChg);

    retval.push_back(xmlrpc_c::value_struct(structVal));
    }

*retvalPtr = xmlrpc_c::value_array(retval);
}
