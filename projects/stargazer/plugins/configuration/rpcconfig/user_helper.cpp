#include "user_helper.h"

#include "../../../tariffs.h"
#include "../../../admin.h"
#include "base_store.h"
#include "user_ips.h"
#include "utils.h"
#include "common.h"

//------------------------------------------------------------------------------

void USER_HELPER::GetUserInfo(xmlrpc_c::value * info,
                              bool hidePassword)
{
std::string enc;

std::map<std::string, xmlrpc_c::value> structVal;

structVal["result"] = xmlrpc_c::value_boolean(true);
structVal["login"] = xmlrpc_c::value_string(iter->GetLogin());

if (!hidePassword)
    {
    structVal["password"] = xmlrpc_c::value_string(iter->property.password.Get());
    }
else
    {
    structVal["password"] = xmlrpc_c::value_string("++++++++");
    }

structVal["cash"] = xmlrpc_c::value_double(iter->property.cash.Get());
structVal["freemb"] = xmlrpc_c::value_double(iter->property.freeMb.Get());
structVal["credit"] = xmlrpc_c::value_double(iter->property.credit.Get());

if (iter->property.nextTariff.Get() != "")
    {
    structVal["tariff"] = xmlrpc_c::value_string(
            iter->property.tariffName.Get() +
            "/" +
            iter->property.nextTariff.Get()
            );
    }
else
    {
    structVal["tariff"] = xmlrpc_c::value_string(iter->property.tariffName.Get());
    }

structVal["note"] = xmlrpc_c::value_string(IconvString(iter->property.note, "KOI8-R", "UTF-8"));

structVal["phone"] = xmlrpc_c::value_string(IconvString(iter->property.phone, "KOI8-R", "UTF-8"));

structVal["address"] = xmlrpc_c::value_string(IconvString(iter->property.address, "KOI8-R", "UTF-8"));

structVal["email"] = xmlrpc_c::value_string(IconvString(iter->property.email, "KOI8-R", "UTF-8"));

std::vector<xmlrpc_c::value> userdata;

userdata.push_back(xmlrpc_c::value_string(IconvString(iter->property.userdata0.Get(), "KOI8-R", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(iter->property.userdata1.Get(), "KOI8-R", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(iter->property.userdata2.Get(), "KOI8-R", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(iter->property.userdata3.Get(), "KOI8-R", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(iter->property.userdata4.Get(), "KOI8-R", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(iter->property.userdata5.Get(), "KOI8-R", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(iter->property.userdata6.Get(), "KOI8-R", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(iter->property.userdata7.Get(), "KOI8-R", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(iter->property.userdata8.Get(), "KOI8-R", "UTF-8")));
userdata.push_back(xmlrpc_c::value_string(IconvString(iter->property.userdata9.Get(), "KOI8-R", "UTF-8")));

structVal["userdata"] = xmlrpc_c::value_array(userdata);

structVal["name"] = xmlrpc_c::value_string(IconvString(iter->property.realName, "KOI8-R", "UTF-8"));

structVal["group"] = xmlrpc_c::value_string(IconvString(iter->property.group, "KOI8-R", "UTF-8"));

structVal["status"] = xmlrpc_c::value_boolean(iter->GetConnected());
structVal["aonline"] = xmlrpc_c::value_boolean(iter->property.alwaysOnline.Get());
structVal["currip"] = xmlrpc_c::value_string(inet_ntostring(iter->GetCurrIP()));
structVal["pingtime"] = xmlrpc_c::value_int(iter->GetPingTime());
structVal["ips"] = xmlrpc_c::value_string(iter->property.ips.Get().GetIpStr());

std::map<std::string, xmlrpc_c::value> traffInfo;
std::vector<xmlrpc_c::value> mu(DIR_NUM);
std::vector<xmlrpc_c::value> md(DIR_NUM);
std::vector<xmlrpc_c::value> su(DIR_NUM);
std::vector<xmlrpc_c::value> sd(DIR_NUM);

DIR_TRAFF upload;
DIR_TRAFF download;
DIR_TRAFF supload;
DIR_TRAFF sdownload;
download = iter->property.down.Get();
upload = iter->property.up.Get();
sdownload = iter->GetSessionUpload();
supload = iter->GetSessionDownload();

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

structVal["down"] = xmlrpc_c::value_boolean(iter->property.disabled.Get());
structVal["disableddetailstat"] = xmlrpc_c::value_boolean(iter->property.disabledDetailStat.Get());
structVal["passive"] = xmlrpc_c::value_boolean(iter->property.passive.Get());
structVal["lastcash"] = xmlrpc_c::value_double(iter->property.lastCashAdd.Get());
structVal["lasttimecash"] = xmlrpc_c::value_int(iter->property.lastCashAddTime.Get());
structVal["lastactivitytime"] = xmlrpc_c::value_int(iter->property.lastActivityTime.Get());
structVal["creditexpire"] = xmlrpc_c::value_int(iter->property.creditExpire.Get());

*info = xmlrpc_c::value_struct(structVal);
}

//------------------------------------------------------------------------------

bool USER_HELPER::SetUserInfo(const xmlrpc_c::value & info,
                              const ADMIN & admin,
                              const std::string & login,
                              const BASE_STORE & store,
                              TARIFFS * tariffs)
{
std::map<std::string, xmlrpc_c::value> structVal(
    static_cast<std::map<std::string, xmlrpc_c::value> >(xmlrpc_c::value_struct(info))
    );

std::map<std::string, xmlrpc_c::value>::iterator it;

if ((it = structVal.find("password")) != structVal.end())
    {
    std::string value(xmlrpc_c::value_string(it->second));
    if (iter->property.password.Get() != value)
        if (!iter->property.password.Set(value,
                                         admin,
                                         login,
                                         &store))
            return true;
    }

if ((it = structVal.find("ips")) != structVal.end())
    {
    USER_IPS ips;
    ips = StrToIPS(xmlrpc_c::value_string(it->second));
    if (!iter->property.ips.Set(ips,
                                admin,
                                login,
                                &store))
        return true;
    }

if ((it = structVal.find("address")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-R"));
    if (iter->property.address.Get() != value)
        if (!iter->property.address.Set(value,
                                        admin,
                                        login,
                                        &store))
            return true;
    }

if ((it = structVal.find("phone")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-R"));
    if (iter->property.phone.Get() != value)
        if (!iter->property.phone.Set(value,
                                      admin,
                                      login,
                                      &store))
            return true;
    }

if ((it = structVal.find("email")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-R"));
    if (iter->property.email.Get() != value)
        if (!iter->property.email.Set(value,
                                      admin,
                                      login,
                                      &store))
            return true;
    }

if ((it = structVal.find("cash")) != structVal.end())
    {
    double value(xmlrpc_c::value_double(it->second));
    if (iter->property.cash.Get() != value)
        if (!iter->property.cash.Set(value,
                                     admin,
                                     login,
                                     &store))
            return true;
    }

if ((it = structVal.find("creditexpire")) != structVal.end())
    {
    time_t value(xmlrpc_c::value_int(it->second));
    if (iter->property.creditExpire.Get() != value)
        if (!iter->property.creditExpire.Set(value,
                                             admin,
                                             login,
                                             &store))
            return true;
    }

if ((it = structVal.find("credit")) != structVal.end())
    {
    double value(xmlrpc_c::value_double(it->second));
    if (iter->property.credit.Get() != value)
        if (!iter->property.credit.Set(value,
                                       admin,
                                       login,
                                       &store))
            return true;
    }

if ((it = structVal.find("freemb")) != structVal.end())
    {
    double value(xmlrpc_c::value_double(it->second));
    if (iter->property.freeMb.Get() != value)
        if (!iter->property.freeMb.Set(value,
                                       admin,
                                       login,
                                       &store))
            return true;
    }

if ((it = structVal.find("down")) != structVal.end())
    {
    bool value(xmlrpc_c::value_boolean(it->second));
    if (iter->property.disabled.Get() != value)
        if (!iter->property.disabled.Set(value,
                                         admin,
                                         login,
                                         &store))
            return true;
    }

if ((it = structVal.find("passive")) != structVal.end())
    {
    bool value(xmlrpc_c::value_boolean(it->second));
    if (iter->property.passive.Get() != value)
        if (!iter->property.passive.Set(value,
                                        admin,
                                        login,
                                        &store))
            return true;
    }

if ((it = structVal.find("aonline")) != structVal.end())
    {
    bool value(xmlrpc_c::value_boolean(it->second));
    if (iter->property.alwaysOnline.Get() != value)
        if (!iter->property.alwaysOnline.Set(value,
                                             admin,
                                             login,
                                             &store))
            return true;
    }

if ((it = structVal.find("disableddetailstat")) != structVal.end())
    {
    bool value(xmlrpc_c::value_boolean(it->second));
    if (iter->property.disabledDetailStat.Get() != value)
        if (!iter->property.disabledDetailStat.Set(value,
                                                   admin,
                                                   login,
                                                   &store))
            return true;
    }

if ((it = structVal.find("name")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-R"));
    if (iter->property.realName.Get() != value)
        if (!iter->property.realName.Set(value,
                                         admin,
                                         login,
                                         &store))
            return true;
    }

if ((it = structVal.find("group")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-R"));
    if (iter->property.group.Get() != value)
        if (!iter->property.group.Set(value,
                                      admin,
                                      login,
                                      &store))
            return true;
    }

if ((it = structVal.find("note")) != structVal.end())
    {
    std::string value(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-R"));
    if (iter->property.note.Get() != value)
        if (!iter->property.note.Set(value,
                                     admin,
                                     login,
                                     &store))
            return true;
    }

if ((it = structVal.find("userdata")) != structVal.end())
    {
    std::vector<USER_PROPERTY_LOGGED<string> *> userdata;
    userdata.push_back(iter->property.userdata0.GetPointer());
    userdata.push_back(iter->property.userdata1.GetPointer());
    userdata.push_back(iter->property.userdata2.GetPointer());
    userdata.push_back(iter->property.userdata3.GetPointer());
    userdata.push_back(iter->property.userdata4.GetPointer());
    userdata.push_back(iter->property.userdata5.GetPointer());
    userdata.push_back(iter->property.userdata6.GetPointer());
    userdata.push_back(iter->property.userdata7.GetPointer());
    userdata.push_back(iter->property.userdata8.GetPointer());
    userdata.push_back(iter->property.userdata9.GetPointer());

    std::vector<xmlrpc_c::value> udata(
        xmlrpc_c::value_array(it->second).vectorValueValue()
        );

    for (unsigned i = 0; i < userdata.size(); ++i)
        {
        std::string value(IconvString(xmlrpc_c::value_string(udata[i]), "UTF-8", "KOI8-R"));
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
    dtData = iter->property.up.Get();
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
        if (!iter->property.up.Set(dtData,
                                   admin,
                                   login,
                                   &store))
            return true;
        }
    dtData = iter->property.down.Get();
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
        if (!iter->property.down.Set(dtData,
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

    if (tariffs->FindByName(tariff))
        if (iter->property.tariffName.Get() != tariff)
            if (!iter->property.tariffName.Set(tariff,
                                               admin,
                                               login,
                                               &store))
                return true;

    if (nextTariff != "" &&
        tariffs->FindByName(nextTariff))
        if (iter->property.nextTariff.Get() != nextTariff)
            if (!iter->property.nextTariff.Set(tariff,
                                               admin,
                                               login,
                                               &store))
                return true;
    }

return false;
}
