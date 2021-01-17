#include <ostream> // xmlrpc-c devs have missed something :)

#include "tariffs_methods.h"
#include "rpcconfig.h"
#include "tariff_helper.h"

#include "stg/tariffs.h"
#include "stg/tariff.h"
#include "stg/tariff_conf.h"
#include "stg/users.h"
#include "stg/admins.h"
#include "stg/admin.h"

void METHOD_TARIFF_GET::execute(xmlrpc_c::paramList const & paramList,
                                xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
std::string name = paramList.getString(1);
paramList.verifyEnd(2);

std::map<std::string, xmlrpc_c::value> structVal;
ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    structVal["result"] = xmlrpc_c::value_boolean(false);
    *retvalPtr = xmlrpc_c::value_struct(structVal);
    return;
    }

const auto tariff = tariffs->FindByName(name);

if (!tariff)
    {
    structVal["result"] = xmlrpc_c::value_boolean(false);
    *retvalPtr = xmlrpc_c::value_struct(structVal);
    return;
    }

auto td = tariff->GetTariffData();

TARIFF_HELPER helper(td);

helper.GetTariffInfo(retvalPtr);
}

void METHOD_TARIFF_CHG::execute(xmlrpc_c::paramList const & paramList,
                                xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
std::string name = paramList.getString(1);
xmlrpc_c::value_struct info(paramList.getStruct(2));
paramList.verifyEnd(3);

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

const auto tariff = tariffs->FindByName(name);

if (!tariff)
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

auto td = tariff->GetTariffData();

TARIFF_HELPER helper(td);

helper.SetTariffInfo(info);

if (tariffs->Chg(td, admin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

*retvalPtr = xmlrpc_c::value_boolean(true);
}

void METHOD_TARIFFS_GET::execute(xmlrpc_c::paramList const & paramList,
                                 xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
paramList.verifyEnd(1);

std::map<std::string, xmlrpc_c::value> structVal;
ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    structVal["result"] = xmlrpc_c::value_boolean(false);
    *retvalPtr = xmlrpc_c::value_struct(structVal);
    return;
    }

std::vector<xmlrpc_c::value> tariffsInfo;


std::vector<STG::TariffData> dataList;
tariffs->GetTariffsData(&dataList);
auto it = dataList.begin();
for (; it != dataList.end(); ++it)
    {
    xmlrpc_c::value info;
    auto td = *it; // 'cause TARIFF_HELPER work in both ways and take not const referense
    TARIFF_HELPER helper(td);
    helper.GetTariffInfo(&info);
    tariffsInfo.push_back(info);
    }

*retvalPtr = xmlrpc_c::value_array(tariffsInfo);
}

void METHOD_TARIFF_ADD::execute(xmlrpc_c::paramList const & paramList,
                                xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
std::string tariff = paramList.getString(1);
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

if (tariffs->Add(tariff, admin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

*retvalPtr = xmlrpc_c::value_boolean(true);
}

void METHOD_TARIFF_DEL::execute(xmlrpc_c::paramList const & paramList,
                                xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
std::string tariff = paramList.getString(1);
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

if (users->TariffInUse(tariff))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

if (tariffs->Del(tariff, admin))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

*retvalPtr = xmlrpc_c::value_boolean(true);
}
