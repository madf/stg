#include <cmath>

#include "stg/tariffs.h"
#include "stg/admin.h"
#include "stg/store.h"
#include "stg/user_ips.h"
#include "stg/common.h"
#include "stg/user_property.h"
#include "user_helper.h"

//------------------------------------------------------------------------------

void USER_HELPER::GetUserInfo(xmlrpc_c::value * info,
                              bool hidePassword)
{
std::map<std::string, xmlrpc_c::value> structVal;

structVal["result"] = xmlrpc_c::value_boolean(true);
structVal["login"] = xmlrpc_c::value_string(ptr->GetLogin());

if (!hidePassword)
    {
    structVal["password"] = xmlrpc_c::value_string(ptr->GetProperty().password.Get());
    }
else
    {
    structVal["password"] = xmlrpc_c::value_string("++++++++");
    }

structVal["cash"] = xmlrpc_c::value_double(ptr->GetProperty().cash.Get());
structVal["freemb"] = xmlrpc_c::value_double(ptr->GetProperty().freeMb.Get());
structVal["credit"] = xmlrpc_c::value_double(ptr->GetProperty().credit.Get());

if (ptr->GetProperty().nextTariff.Get() != "")
    {
    structVal["tariff"] = xmlrpc_c::value_string(
            ptr->GetProperty().tariffName.Get() +
            "/" +
            ptr->GetProperty().nextTariff.Get()
            );
    }
else
    {
    structVal["tariff"] = xmlrpc_c::value_string(ptr->GetProperty().tariffName.Get());
    }

structVal["note"] = xmlrpc_c::value_string(IconvString(ptr->GetProperty().note, "KOI8-RU", "UTF-8"));

structVal["phone"] = xmlrpc_c::value_string(IconvString(ptr->GetProperty().phone, "KOI8-RU", "UTF-8"));

structVal["address"] = xmlrpc_c::value_string(IconvString(ptr->GetProperty().address, "KOI8-RU", "UTF-8"));

structVal["email"] = xmlrpc_c::value_string(IconvString(ptr->GetProperty().email, "KOI8-RU", "UTF-8"));

std::vector<xmlrpc_c::value> userdata;

userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperty().userdata0.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperty().userdata1.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperty().userdata2.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperty().userdata3.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperty().userdata4.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperty().userdata5.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperty().userdata6.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperty().userdata7.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperty().userdata8.Get(), "KOI8-RU", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(ptr->GetProperty().userdata9.Get(), "KOI8-RU", "UTF-8")));

structVal["userdata"] = xmlrpc_c::value_array(userdata);

structVal["name"] = xmlrpc_c::value_string(IconvString(ptr->GetProperty().realName, "KOI8-RU", "UTF-8"));

structVal["group"] = xmlrpc_c::value_string(IconvString(ptr->GetProperty().group, "KOI8-RU", "UTF-8"));

structVal["status"] = xmlrpc_c::value_boolean(ptr->GetConnected());
structVal["aonline"] = xmlrpc_c::value_boolean(ptr->GetProperty().alwaysOnline.Get());
structVal["currip"] = xmlrpc_c::value_string(inet_ntostring(ptr->GetCurrIP()));
structVal["pingtime"] = xmlrpc_c::value_int(static_cast<int>(ptr->GetPingTime()));
structVal["ips"] = xmlrpc_c::value_string(ptr->GetProperty().ips.Get().GetIpStr());

std::map<std::string, xmlrpc_c::value> traffInfo;
std::vector<xmlrpc_c::value> mu(DIR_NUM);
std::vector<xmlrpc_c::value> md(DIR_NUM);
std::vector<xmlrpc_c::value> su(DIR_NUM);
std::vector<xmlrpc_c::value> sd(DIR_NUM);

DIR_TRAFF upload;
DIR_TRAFF download;
DIR_TRAFF supload;
DIR_TRAFF sdownload;
download = ptr->GetProperty().down.Get();
upload = ptr->GetProperty().up.Get();
sdownload = ptr->GetSessionUpload();
supload = ptr->GetSessionDownload();

for (int j = 0; j < DIR_NUM; j++)
    {
    std::string value;
    x2str(upload[j], value);
    mu[j] = xmlrpc_c::value_string(value);
    x2str(download[j], value);
    md[j] = xmlrpc_c::value_string(value);
    x2str(supload[j], value);
    su[j] = xmlrpc_c::value_string(value);
    x2str(sdownload[j], value);
    sd[j] = xmlrpc_c::value_string(value);
    }

traffInfo["mu"] = xmlrpc_c::value_array(mu);
traffInfo["md"] = xmlrpc_c::value_array(md);
traffInfo["su"] = xmlrpc_c::value_array(su);
traffInfo["sd"] = xmlrpc_c::value_array(sd);

structVal["traff"] = xmlrpc_c::value_struct(traffInfo);

structVal["down"] = xmlrpc_c::value_boolean(ptr->GetProperty().disabled.Get());
structVal["disableddetailstat"] = xmlrpc_c::value_boolean(ptr->GetProperty().disabledDetailStat.Get());
structVal["passive"] = xmlrpc_c::value_boolean(ptr->GetProperty().passive.Get());
structVal["lastcash"] = xmlrpc_c::value_double(ptr->GetProperty().lastCashAdd.Get());
structVal["lasttimecash"] = xmlrpc_c::value_int(static_cast<int>(ptr->GetProperty().lastCashAddTime.Get()));
structVal["lastactivitytime"] = xmlrpc_c::value_int(static_cast<int>(ptr->GetProperty().lastActivityTime.Get()));
structVal["creditexpire"] = xmlrpc_c::value_int(static_cast<int>(ptr->GetProperty().creditExpire.Get()));

*info = xmlrpc_c::value_struct(structVal);
}

//------------------------------------------------------------------------------

bool USER_HELPER::SetUserInfo(const xmlrpc_c::value & info,
                              const ADMIN * admin,
                              const std::string & login,
                              const STORE & store,
                              TARIFFS * tariffs)
{
std::map<std::string, xmlrpc_c::value> structVal(
    static_cast<std::map<std::string, xmlrpc_c::value> >(xmlrpc_c::value_struct(info))
    );

std::map<std::string, xmlrpc_c::value>::iterator it;

bool check = false;
bool alwaysOnline = ptr->GetProperty().alwaysOnline;
if ((it = structVal.find("aonline")) != structVal.end())
    {
    check = true;
    alwaysOnline = xmlrpc_c::value_boolean(it->second);
    }
bool onlyOneIP = ptr->GetProperty().ips.ConstData().OnlyOneIP();
if ((it = structVal.find("ips")) != structVal.end())
    {
    check = true;
    onlyOneIP = StrToIPS(xmlrpc_c::value_string(it->second)).OnlyOneIP();
    }

if (check && alwaysOnline && !onlyOneIP)
    {
    printfd(__FILE__, "Requested change leads to a forbidden state: AlwaysOnline with multiple IP's\n");
    return true;
    }

if ((it = structVal.find("ips")) != structVal.end())
    {
    USER_IPS ips;
    ips = StrToIPS(xmlrpc_c::value_string(it->second));

    for (size_t i = 0; i < ips.Count(); ++i)
        {
        CONST_USER_PTR user;
        uint32_t ip = ips[i].ip;
        if (users.IsIPInUse(ip, login, &user))
            {
            printfd(__FILE__, "Trying to assign an IP %s to '%s' that is already in use by '%s'\n", inet_ntostring(ip).c_str(), login.c_str(), user->GetLogin().c_str());
            return true;
            }
        }

    if (!ptr->GetProperty().ips.Set(ips,
                                admin,
                                login,
                                &store))
        return true;
    }

if ((it = structVal.find("aonline")) != structVal.end())
    {
    bool value(xmlrpc_c::value_boolean(it->second));
    if (ptr->GetProperty().alwaysOnline.Get() != value)
        if (!ptr->GetProperty().alwaysOnline.Set(value,
                                             admin,
                                             login,
                                             &store))
            return true;
    }

if ((it = structVal.find("password")) != structVal.end())
    {
    std::string value(xmlrpc_c::value_string(it->second));
    if (ptr->GetProperty().password.Get() != value)
        if (!ptr->GetProperty().password.Set(value,
                                         admin,
                                         login,
                                         &store))
            return true;
    }

if ((it = structVal.find("address")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-RU"));
    if (ptr->GetProperty().address.Get() != value)
        if (!ptr->GetProperty().address.Set(value,
                                        admin,
                                        login,
                                        &store))
            return true;
    }

if ((it = structVal.find("phone")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-RU"));
    if (ptr->GetProperty().phone.Get() != value)
        if (!ptr->GetProperty().phone.Set(value,
                                      admin,
                                      login,
                                      &store))
            return true;
    }

if ((it = structVal.find("email")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-RU"));
    if (ptr->GetProperty().email.Get() != value)
        if (!ptr->GetProperty().email.Set(value,
                                      admin,
                                      login,
                                      &store))
            return true;
    }

if ((it = structVal.find("cash")) != structVal.end())
    {
    double value(xmlrpc_c::value_double(it->second));
    if (std::fabs(ptr->GetProperty().cash.Get() - value) > 1.0e-3)
        if (!ptr->GetProperty().cash.Set(value,
                                     admin,
                                     login,
                                     &store))
            return true;
    }

if ((it = structVal.find("creditexpire")) != structVal.end())
    {
    time_t value(xmlrpc_c::value_int(it->second));
    if (ptr->GetProperty().creditExpire.Get() != value)
        if (!ptr->GetProperty().creditExpire.Set(value,
                                             admin,
                                             login,
                                             &store))
            return true;
    }

if ((it = structVal.find("credit")) != structVal.end())
    {
    double value(xmlrpc_c::value_double(it->second));
    if (std::fabs(ptr->GetProperty().credit.Get() - value) > 1.0e-3)
        if (!ptr->GetProperty().credit.Set(value,
                                       admin,
                                       login,
                                       &store))
            return true;
    }

if ((it = structVal.find("freemb")) != structVal.end())
    {
    double value(xmlrpc_c::value_double(it->second));
    if (std::fabs(ptr->GetProperty().freeMb.Get() - value) > 1.0e-3)
        if (!ptr->GetProperty().freeMb.Set(value,
                                       admin,
                                       login,
                                       &store))
            return true;
    }

if ((it = structVal.find("down")) != structVal.end())
    {
    bool value(xmlrpc_c::value_boolean(it->second));
    if (ptr->GetProperty().disabled.Get() != value)
        if (!ptr->GetProperty().disabled.Set(value,
                                         admin,
                                         login,
                                         &store))
            return true;
    }

if ((it = structVal.find("passive")) != structVal.end())
    {
    bool value(xmlrpc_c::value_boolean(it->second));
    if (ptr->GetProperty().passive.Get() != value)
        if (!ptr->GetProperty().passive.Set(value,
                                        admin,
                                        login,
                                        &store))
            return true;
    }

if ((it = structVal.find("disableddetailstat")) != structVal.end())
    {
    bool value(xmlrpc_c::value_boolean(it->second));
    if (ptr->GetProperty().disabledDetailStat.Get() != value)
        if (!ptr->GetProperty().disabledDetailStat.Set(value,
                                                   admin,
                                                   login,
                                                   &store))
            return true;
    }

if ((it = structVal.find("name")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-RU"));
    if (ptr->GetProperty().realName.Get() != value)
        if (!ptr->GetProperty().realName.Set(value,
                                         admin,
                                         login,
                                         &store))
            return true;
    }

if ((it = structVal.find("group")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-RU"));
    if (ptr->GetProperty().group.Get() != value)
        if (!ptr->GetProperty().group.Set(value,
                                      admin,
                                      login,
                                      &store))
            return true;
    }

if ((it = structVal.find("note")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-RU"));
    if (ptr->GetProperty().note.Get() != value)
        if (!ptr->GetProperty().note.Set(value,
                                     admin,
                                     login,
                                     &store))
            return true;
    }

if ((it = structVal.find("userdata")) != structVal.end())
    {
    std::vector<USER_PROPERTY_LOGGED<std::string> *> userdata;
    userdata.push_back(ptr->GetProperty().userdata0.GetPointer());
    userdata.push_back(ptr->GetProperty().userdata1.GetPointer());
    userdata.push_back(ptr->GetProperty().userdata2.GetPointer());
    userdata.push_back(ptr->GetProperty().userdata3.GetPointer());
    userdata.push_back(ptr->GetProperty().userdata4.GetPointer());
    userdata.push_back(ptr->GetProperty().userdata5.GetPointer());
    userdata.push_back(ptr->GetProperty().userdata6.GetPointer());
    userdata.push_back(ptr->GetProperty().userdata7.GetPointer());
    userdata.push_back(ptr->GetProperty().userdata8.GetPointer());
    userdata.push_back(ptr->GetProperty().userdata9.GetPointer());

    std::vector<xmlrpc_c::value> udata(
        xmlrpc_c::value_array(it->second).vectorValueValue()
        );

    for (unsigned i = 0; i < userdata.size(); ++i)
        {
        std::string value(IconvString(xmlrpc_c::value_string(udata[i]), "UTF-8", "KOI8-RU"));
        if (userdata[i]->Get() != value)
            if (!userdata[i]->Set(value,
                                  admin,
                                  login,
                                  &store))
                return true;
        }
    }

if ((it = structVal.find("traff")) != structVal.end())
    {
    std::map<std::string, xmlrpc_c::value> traff(
        static_cast<std::map<std::string, xmlrpc_c::value> >(xmlrpc_c::value_struct(it->second))
        );

    DIR_TRAFF dtData;
    dtData = ptr->GetProperty().up.Get();
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
        if (!ptr->GetProperty().up.Set(dtData,
                                   admin,
                                   login,
                                   &store))
            return true;
        }
    dtData = ptr->GetProperty().down.Get();
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
        if (!ptr->GetProperty().down.Set(dtData,
                                     admin,
                                     login,
                                     &store))
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

    const TARIFF * newTariff = tariffs->FindByName(tariff);
    if (newTariff)
        {
        const TARIFF * currentTariff = ptr->GetTariff();
        std::string message = currentTariff->TariffChangeIsAllowed(*newTariff, stgTime);
        if (message.empty())
            {
            if (ptr->GetProperty().tariffName.Get() != tariff)
                {
                if (!ptr->GetProperty().tariffName.Set(tariff,
                                                   admin,
                                                   login,
                                                   &store))
                    return true;
                }
            }
        else
            {
            GetStgLogger()("Tariff change is prohibited for user %s. %s", ptr->GetLogin().c_str(), message.c_str());
            }
        }

    if (nextTariff != "" &&
        tariffs->FindByName(nextTariff))
        if (ptr->GetProperty().nextTariff.Get() != nextTariff)
            if (!ptr->GetProperty().nextTariff.Set(tariff,
                                               admin,
                                               login,
                                               &store))
                return true;
    }

return false;
}
