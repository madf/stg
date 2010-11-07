#include "user_helper.h"

#include "user_ips.h"
#include "utils.h"

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
                              const BASE_STORE & store)
{
std::map<std::string, xmlrpc_c::value> structVal(
    static_cast<std::map<std::string, xmlrpc_c::value> >(xmlrpc_c::value_struct(info))
    );

std::map<std::string, xmlrpc_c::value>::iterator it;

if ((it = structVal.find("password")) != structVal.end())
    {
    bool res = iter->property.password.Set(xmlrpc_c::value_string(it->second),
                                           admin,
                                           login,
                                           &store);
    if (!res)
        {
        return true;
        }
    }

if ((it = structVal.find("ips")) != structVal.end())
    {
    USER_IPS ips;
    ips = StrToIPS(xmlrpc_c::value_string(it->second));
    bool res = iter->property.ips.Set(ips,
                                      admin,
                                      login,
                                      &store);
    if (!res)
        {
        return true;
        }
    }

if ((it = structVal.find("address")) != structVal.end())
    {
    bool res = iter->property.address.Set(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-R"),
                                          admin,
                                          login,
                                          &store);
    if (!res)
        {
        return true;
        }
    }

if ((it = structVal.find("phone")) != structVal.end())
    {
    bool res = iter->property.phone.Set(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-R"),
                                        admin,
                                        login,
                                        &store);
    if (!res)
        {
        return true;
        }
    }

if ((it = structVal.find("email")) != structVal.end())
    {
    bool res = iter->property.email.Set(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-R"),
                                        admin,
                                        login,
                                        &store);
    if (!res)
        {
        return true;
        }
    }

if ((it = structVal.find("creditexpire")) != structVal.end())
    {
    bool res = iter->property.creditExpire.Set(xmlrpc_c::value_int(it->second),
                                               admin,
                                               login,
                                               &store);
    if (!res)
        {
        return true;
        }
    }

if ((it = structVal.find("credit")) != structVal.end())
    {
    bool res = iter->property.credit.Set(xmlrpc_c::value_double(it->second),
                                         admin,
                                         login,
                                         &store);
    if (!res)
        {
        return true;
        }
    }

if ((it = structVal.find("freemb")) != structVal.end())
    {
    bool res = iter->property.freeMb.Set(xmlrpc_c::value_double(it->second),
                                         admin,
                                         login,
                                         &store);
    if (!res)
        {
        return true;
        }
    }

if ((it = structVal.find("disabled")) != structVal.end())
    {
    bool res = iter->property.disabled.Set(xmlrpc_c::value_boolean(it->second),
                                           admin,
                                           login,
                                           &store);
    if (!res)
        {
        return true;
        }
    }

if ((it = structVal.find("passive")) != structVal.end())
    {
    bool res = iter->property.passive.Set(xmlrpc_c::value_boolean(it->second),
                                          admin,
                                          login,
                                          &store);
    if (!res)
        {
        return true;
        }
    }

if ((it = structVal.find("aonline")) != structVal.end())
    {
    bool res = iter->property.alwaysOnline.Set(xmlrpc_c::value_boolean(it->second),
                                               admin,
                                               login,
                                               &store);
    if (!res)
        {
        return true;
        }
    }

if ((it = structVal.find("disableddetailstat")) != structVal.end())
    {
    bool res = iter->property.disabledDetailStat.Set(xmlrpc_c::value_boolean(it->second),
                                                     admin,
                                                     login,
                                                     &store);
    if (!res)
        {
        return true;
        }
    }

if ((it = structVal.find("name")) != structVal.end())
    {
    bool res = iter->property.realName.Set(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-R"),
                                           admin,
                                           login,
                                           &store);
    if (!res)
        {
        return true;
        }
    }

if ((it = structVal.find("group")) != structVal.end())
    {
    bool res = iter->property.group.Set(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-R"),
                                        admin,
                                        login,
                                        &store);
    if (!res)
        {
        return true;
        }
    }

if ((it = structVal.find("note")) != structVal.end())
    {
    bool res = iter->property.note.Set(IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "KOI8-R"),
                                       admin,
                                       login,
                                       &store);
    if (!res)
        {
        return true;
        }
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
        bool res = userdata[i]->Set(IconvString(xmlrpc_c::value_string(udata[i]), "UTF-8", "KOI8-R"),
                                    admin,
                                    login,
                                    &store);
        if (!res)
            {
            return true;
            }
        }
    }

if ((it = structVal.find("traff")) != structVal.end())
    {
    std::map<std::string, xmlrpc_c::value> traff(
        static_cast<std::map<std::string, xmlrpc_c::value> >(xmlrpc_c::value_struct(it->second))
        );

    std::vector<xmlrpc_c::value> data;
    DIR_TRAFF dtData;
    dtData = iter->property.up.Get();
    if ((it = traff.find("mu")) != traff.end())
        {
        data = xmlrpc_c::value_array(it->second).vectorValueValue();

        for (int i = 0; i < std::min(DIR_NUM, static_cast<int>(data.size())); ++i)
            {
            int64_t value;
            if (str2x(xmlrpc_c::value_string(data[i]), value))
                {
                printfd(__FILE__, "USER_HELPER::SetUserInfo(): 'Invalid month upload value'\n");
                }
            else
                {
                dtData[i] = value;
                }
            }
        bool res = iter->property.up.Set(dtData,
                                         admin,
                                         login,
                                         &store);
        if (!res)
            {
            return true;
            }
        }
    dtData = iter->property.down.Get();
    if ((it = traff.find("md")) != traff.end())
        {
        data = xmlrpc_c::value_array(it->second).vectorValueValue();

        for (int i = 0; i < std::min(DIR_NUM, static_cast<int>(data.size())); ++i)
            {
            int64_t value;
            if (str2x(xmlrpc_c::value_string(data[i]), value))
                {
                printfd(__FILE__, "USER_HELPER::SetUserInfo(): 'Invalid month download value'\n");
                }
            else
                {
                dtData[i] = value;
                }
            }
        bool res = iter->property.down.Set(dtData,
                                           admin,
                                           login,
                                           &store);
        if (!res)
            {
            return true;
            }
        }
    }

return false;
}
