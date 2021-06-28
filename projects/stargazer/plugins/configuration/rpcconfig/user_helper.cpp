#include "user_helper.h"

#include "stg/tariffs.h"
#include "stg/tariff.h"
#include "stg/admin.h"
#include "stg/store.h"
#include "stg/users.h"
#include "stg/user.h"
#include "stg/user_ips.h"
#include "stg/user_property.h"
#include "stg/common.h"
#include "stg/const.h"

#include <cmath>

//------------------------------------------------------------------------------

void USER_HELPER::GetUserInfo(xmlrpc_c::value * info,
                              bool hidePassword)
{
std::map<std::string, xmlrpc_c::value> structVal;

structVal["result"] = xmlrpc_c::value_boolean(true);
structVal["login"] = xmlrpc_c::value_string(ptr->GetLogin());

if (!hidePassword)
    {
    structVal["password"] = xmlrpc_c::value_string(ptr->GetProperties().password.Get());
    }
else
    {
    structVal["password"] = xmlrpc_c::value_string("++++++++");
    }

structVal["cash"] = xmlrpc_c::value_double(ptr->GetProperties().cash.Get());
structVal["freemb"] = xmlrpc_c::value_double(ptr->GetProperties().freeMb.Get());
structVal["credit"] = xmlrpc_c::value_double(ptr->GetProperties().credit.Get());

if (ptr->GetProperties().nextTariff.Get() != "")
    {
    structVal["tariff"] = xmlrpc_c::value_string(
            ptr->GetProperties().tariffName.Get() +
            "/" +
            ptr->GetProperties().nextTariff.Get()
            );
    }
else
    {
    structVal["tariff"] = xmlrpc_c::value_string(ptr->GetProperties().tariffName.Get());
    }

structVal["note"] = xmlrpc_c::value_string(IconvString(ptr->GetProperties().note, "KOI8-RU", "UTF-8"));

structVal["phone"] = xmlrpc_c::value_string(IconvString(ptr->GetProperties().phone, "KOI8-RU", "UTF-8"));

structVal["address"] = xmlrpc_c::value_string(IconvString(ptr->GetProperties().address, "KOI8-RU", "UTF-8"));

structVal["email"] = xmlrpc_c::value_string(IconvString(ptr->GetProperties().email, "KOI8-RU", "UTF-8"));

std::vector<xmlrpc_c::value> userdata;

userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperties().userdata0.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperties().userdata1.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperties().userdata2.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperties().userdata3.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperties().userdata4.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperties().userdata5.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperties().userdata6.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperties().userdata7.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperties().userdata8.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperties().userdata9.Get(), "KOI8-RU", "UTF-8")));

structVal["userdata"] = xmlrpc_c::value_array(userdata);

structVal["name"] = xmlrpc_c::value_string(IconvString(ptr->GetProperties().realName, "KOI8-RU", "UTF-8"));

structVal["group"] = xmlrpc_c::value_string(IconvString(ptr->GetProperties().group, "KOI8-RU", "UTF-8"));

structVal["status"] = xmlrpc_c::value_boolean(ptr->GetConnected());
structVal["aonline"] = xmlrpc_c::value_boolean(ptr->GetProperties().alwaysOnline.Get());
structVal["currip"] = xmlrpc_c::value_string(inet_ntostring(ptr->GetCurrIP()));
structVal["pingtime"] = xmlrpc_c::value_int(static_cast<int>(ptr->GetPingTime()));
structVal["ips"] = xmlrpc_c::value_string(ptr->GetProperties().ips.Get().toString());

std::map<std::string, xmlrpc_c::value> traffInfo;
std::vector<xmlrpc_c::value> mu(DIR_NUM);
std::vector<xmlrpc_c::value> md(DIR_NUM);
std::vector<xmlrpc_c::value> su(DIR_NUM);
std::vector<xmlrpc_c::value> sd(DIR_NUM);

auto upload = ptr->GetProperties().up.Get();
auto download = ptr->GetProperties().down.Get();
auto supload = ptr->GetSessionUpload();
auto sdownload = ptr->GetSessionDownload();

for (int j = 0; j < DIR_NUM; j++)
    {
    mu[j] = xmlrpc_c::value_string(std::to_string(upload[j]));
    md[j] = xmlrpc_c::value_string(std::to_string(download[j]));
    su[j] = xmlrpc_c::value_string(std::to_string(supload[j]));
    sd[j] = xmlrpc_c::value_string(std::to_string(sdownload[j]));
    }

traffInfo["mu"] = xmlrpc_c::value_array(mu);
traffInfo["md"] = xmlrpc_c::value_array(md);
traffInfo["su"] = xmlrpc_c::value_array(su);
traffInfo["sd"] = xmlrpc_c::value_array(sd);

structVal["traff"] = xmlrpc_c::value_struct(traffInfo);

structVal["down"] = xmlrpc_c::value_boolean(ptr->GetProperties().disabled.Get());
structVal["disableddetailstat"] = xmlrpc_c::value_boolean(ptr->GetProperties().disabledDetailStat.Get());
structVal["passive"] = xmlrpc_c::value_boolean(ptr->GetProperties().passive.Get());
structVal["lastcash"] = xmlrpc_c::value_double(ptr->GetProperties().lastCashAdd.Get());
structVal["lasttimecash"] = xmlrpc_c::value_int(static_cast<int>(ptr->GetProperties().lastCashAddTime.Get()));
structVal["lastactivitytime"] = xmlrpc_c::value_int(static_cast<int>(ptr->GetProperties().lastActivityTime.Get()));
structVal["creditexpire"] = xmlrpc_c::value_int(static_cast<int>(ptr->GetProperties().creditExpire.Get()));

*info = xmlrpc_c::value_struct(structVal);
}

//------------------------------------------------------------------------------

bool USER_HELPER::SetUserInfo(const xmlrpc_c::value & info,
                              const STG::Admin& admin,
                              const std::string & login,
                              const STG::Store & store,
                              STG::Tariffs * tariffs)
{
std::map<std::string, xmlrpc_c::value> structVal(
    static_cast<std::map<std::string, xmlrpc_c::value> >(xmlrpc_c::value_struct(info))
    );

std::map<std::string, xmlrpc_c::value>::iterator it;

bool check = false;
bool alwaysOnline = ptr->GetProperties().alwaysOnline;
if ((it = structVal.find("aonline")) != structVal.end())
    {
    check = true;
    alwaysOnline = xmlrpc_c::value_boolean(it->second);
    }
bool onlyOneIP = ptr->GetProperties().ips.ConstData().onlyOneIP();
if ((it = structVal.find("ips")) != structVal.end())
    {
    check = true;
    onlyOneIP = STG::UserIPs::parse(xmlrpc_c::value_string(it->second)).onlyOneIP();
    }

if (check && alwaysOnline && !onlyOneIP)
    {
    printfd(__FILE__, "Requested change leads to a forbidden state: AlwaysOnline with multiple IP's\n");
    return true;
    }

if ((it = structVal.find("ips")) != structVal.end())
    {
    auto ips = STG::UserIPs::parse(xmlrpc_c::value_string(it->second));

    for (size_t i = 0; i < ips.count(); ++i)
        {
        using ConstUserPtr = const STG::User*;
        ConstUserPtr user;
        uint32_t ip = ips[i].ip;
        if (users.IsIPInUse(ip, login, &user))
            {
            printfd(__FILE__, "Trying to assign an IP %s to '%s' that is already in use by '%s'\n", inet_ntostring(ip).c_str(), login.c_str(), user->GetLogin().c_str());
            return true;
            }
        }

    if (!ptr->GetProperties().ips.Set(ips, admin, login, store))
        return true;
    }

if ((it = structVal.find("aonline")) != structVal.end())
    {
    bool value(xmlrpc_c::value_boolean(it->second));
    if (ptr->GetProperties().alwaysOnline.Get() != value)
        if (!ptr->GetProperties().alwaysOnline.Set(value, admin, login, store))
            return true;
    }

if ((it = structVal.find("password")) != structVal.end())
    {
    std::string value(xmlrpc_c::value_string(it->second));
    if (ptr->GetProperties().password.Get() != value)
        if (!ptr->GetProperties().password.Set(value, admin, login, store))
            return true;
    }

if ((it = structVal.find("address")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-RU"));
    if (ptr->GetProperties().address.Get() != value)
        if (!ptr->GetProperties().address.Set(value, admin, login, store))
            return true;
    }

if ((it = structVal.find("phone")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-RU"));
    if (ptr->GetProperties().phone.Get() != value)
        if (!ptr->GetProperties().phone.Set(value, admin, login, store))
            return true;
    }

if ((it = structVal.find("email")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-RU"));
    if (ptr->GetProperties().email.Get() != value)
        if (!ptr->GetProperties().email.Set(value, admin, login, store))
            return true;
    }

if ((it = structVal.find("cash")) != structVal.end())
    {
    double value(xmlrpc_c::value_double(it->second));
    if (std::fabs(ptr->GetProperties().cash.Get() - value) > 1.0e-3)
        if (!ptr->GetProperties().cash.Set(value, admin, login, store))
            return true;
    }

if ((it = structVal.find("creditexpire")) != structVal.end())
    {
    time_t value(xmlrpc_c::value_int(it->second));
    if (ptr->GetProperties().creditExpire.Get() != value)
        if (!ptr->GetProperties().creditExpire.Set(value, admin, login, store))
            return true;
    }

if ((it = structVal.find("credit")) != structVal.end())
    {
    double value(xmlrpc_c::value_double(it->second));
    if (std::fabs(ptr->GetProperties().credit.Get() - value) > 1.0e-3)
        if (!ptr->GetProperties().credit.Set(value, admin, login, store))
            return true;
    }

if ((it = structVal.find("freemb")) != structVal.end())
    {
    double value(xmlrpc_c::value_double(it->second));
    if (std::fabs(ptr->GetProperties().freeMb.Get() - value) > 1.0e-3)
        if (!ptr->GetProperties().freeMb.Set(value, admin, login, store))
            return true;
    }

if ((it = structVal.find("down")) != structVal.end())
    {
    bool value(xmlrpc_c::value_boolean(it->second));
    if (ptr->GetProperties().disabled.Get() != value)
        if (!ptr->GetProperties().disabled.Set(value, admin, login, store))
            return true;
    }

if ((it = structVal.find("passive")) != structVal.end())
    {
    bool value(xmlrpc_c::value_boolean(it->second));
    if (ptr->GetProperties().passive.Get() != value)
        if (!ptr->GetProperties().passive.Set(value, admin, login, store))
            return true;
    }

if ((it = structVal.find("disableddetailstat")) != structVal.end())
    {
    bool value(xmlrpc_c::value_boolean(it->second));
    if (ptr->GetProperties().disabledDetailStat.Get() != value)
        if (!ptr->GetProperties().disabledDetailStat.Set(value, admin, login, store))
            return true;
    }

if ((it = structVal.find("name")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-RU"));
    if (ptr->GetProperties().realName.Get() != value)
        if (!ptr->GetProperties().realName.Set(value, admin, login, store))
            return true;
    }

if ((it = structVal.find("group")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-RU"));
    if (ptr->GetProperties().group.Get() != value)
        if (!ptr->GetProperties().group.Set(value, admin, login, store))
            return true;
    }

if ((it = structVal.find("note")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-RU"));
    if (ptr->GetProperties().note.Get() != value)
        if (!ptr->GetProperties().note.Set(value, admin, login, store))
            return true;
    }

if ((it = structVal.find("userdata")) != structVal.end())
    {
    std::vector<STG::UserPropertyLogged<std::string> *> userdata;
    userdata.push_back(ptr->GetProperties().userdata0.GetPointer());
    userdata.push_back(ptr->GetProperties().userdata1.GetPointer());
    userdata.push_back(ptr->GetProperties().userdata2.GetPointer());
    userdata.push_back(ptr->GetProperties().userdata3.GetPointer());
    userdata.push_back(ptr->GetProperties().userdata4.GetPointer());
    userdata.push_back(ptr->GetProperties().userdata5.GetPointer());
    userdata.push_back(ptr->GetProperties().userdata6.GetPointer());
    userdata.push_back(ptr->GetProperties().userdata7.GetPointer());
    userdata.push_back(ptr->GetProperties().userdata8.GetPointer());
    userdata.push_back(ptr->GetProperties().userdata9.GetPointer());

    std::vector<xmlrpc_c::value> udata(
        xmlrpc_c::value_array(it->second).vectorValueValue()
        );

    for (unsigned i = 0; i < userdata.size(); ++i)
        {
        std::string value(IconvString(xmlrpc_c::value_string(udata[i]), "UTF-8", "KOI8-RU"));
        if (userdata[i]->Get() != value)
            if (!userdata[i]->Set(value, admin, login, store))
                return true;
        }
    }

if ((it = structVal.find("traff")) != structVal.end())
    {
    std::map<std::string, xmlrpc_c::value> traff(
        static_cast<std::map<std::string, xmlrpc_c::value> >(xmlrpc_c::value_struct(it->second))
        );

    auto dtData = ptr->GetProperties().up.Get();
    if ((it = traff.find("mu")) != traff.end())
        {
        std::vector<xmlrpc_c::value> data(xmlrpc_c::value_array(it->second).vectorValueValue());

        for (int i = 0; i < std::min(DIR_NUM, static_cast<int>(data.size())); ++i)
            {
            int64_t value;
            if (str2x(xmlrpc_c::value_string(data[i]), value))
                printfd(__FILE__, "USER_HELPER::SetUserInfo(): 'Invalid month upload value'\n");
            else
                dtData[i] = value;
            }
        if (!ptr->GetProperties().up.Set(dtData, admin, login, store))
            return true;
        }
    dtData = ptr->GetProperties().down.Get();
    if ((it = traff.find("md")) != traff.end())
        {
        std::vector<xmlrpc_c::value> data(xmlrpc_c::value_array(it->second).vectorValueValue());

        for (int i = 0; i < std::min(DIR_NUM, static_cast<int>(data.size())); ++i)
            {
            int64_t value;
            if (str2x(xmlrpc_c::value_string(data[i]), value))
                printfd(__FILE__, "USER_HELPER::SetUserInfo(): 'Invalid month download value'\n");
            else
                dtData[i] = value;
            }
        if (!ptr->GetProperties().down.Set(dtData, admin, login, store))
            return true;
        }
    }

if ((it = structVal.find("tariff")) != structVal.end())
    {
    std::string tariff(xmlrpc_c::value_string(it->second));
    size_t pos = tariff.find('/');
    std::string nextTariff;
    if (pos != std::string::npos)
        {
        nextTariff = tariff.substr(pos + 1);
        tariff = tariff.substr(0, pos);
        }

    const auto newTariff = tariffs->FindByName(tariff);
    if (newTariff)
        {
        const auto currentTariff = ptr->GetTariff();
        std::string message = currentTariff->TariffChangeIsAllowed(*newTariff, time(NULL));
        if (message.empty())
            {
            if (ptr->GetProperties().tariffName.Get() != tariff)
                {
                if (!ptr->GetProperties().tariffName.Set(tariff, admin, login, store))
                    return true;
                }
            }
        else
            {
                STG::PluginLogger::get("conf_rpc")("Tariff change is prohibited for user %s. %s", ptr->GetLogin().c_str(), message.c_str());
            }
        }

    if (nextTariff != "" &&
        tariffs->FindByName(nextTariff))
        if (ptr->GetProperties().nextTariff.Get() != nextTariff)
            if (!ptr->GetProperties().nextTariff.Set(tariff, admin, login, store))
                return true;
    }

return false;
}
