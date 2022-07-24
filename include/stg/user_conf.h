#pragma once

#include "user_ips.h"

#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include "const.h"

namespace STG
{

//-----------------------------------------------------------------------------
struct UserConf
{
    UserConf() noexcept
        : passive(0),
          disabled(0),
          disabledDetailStat(0),
          alwaysOnline(0),
          credit(0),
          userdata(USERDATA_NUM),
          creditExpire(0)
    {}

    UserConf(const UserConf&) = default;
    UserConf& operator=(const UserConf&) = default;
    UserConf(UserConf&&) = default;
    UserConf& operator=(UserConf&&) = default;

    std::string              password;
    int                      passive;
    int                      disabled;
    int                      disabledDetailStat;
    int                      alwaysOnline;
    std::string              tariffName;
    std::string              address;
    std::string              phone;
    std::string              email;
    std::string              note;
    std::string              realName;
    std::string              corp;
    std::vector<std::string> services;
    std::string              group;
    double                   credit;
    std::string              nextTariff;
    std::vector<std::string> userdata;
    time_t                   creditExpire;
    UserIPs                  ips;
};
//-----------------------------------------------------------------------------
struct UserConfOpt
{
    UserConfOpt() noexcept
        : userdata(USERDATA_NUM)
    {}
    UserConfOpt(const UserConf& data) noexcept
        : password(data.password),
          passive(data.passive),
          disabled(data.disabled),
          disabledDetailStat(data.disabledDetailStat),
          alwaysOnline(data.alwaysOnline),
          tariffName(data.tariffName),
          address(data.address),
          phone(data.phone),
          email(data.email),
          note(data.note),
          realName(data.realName),
          corp(data.corp),
          group(data.group),
          credit(data.credit),
          nextTariff(data.nextTariff),
          userdata(USERDATA_NUM),
          services(data.services),
          creditExpire(data.creditExpire),
          ips(data.ips)
    {
        for (size_t i = 0; i < USERDATA_NUM; i++)
            userdata[i]  = data.userdata[i];
    }
    UserConfOpt& operator=(const UserConf& uc) noexcept
    {
        userdata.resize(USERDATA_NUM);
        password     = uc.password;
        passive      = uc.passive;
        disabled     = uc.disabled;
        disabledDetailStat = uc.disabledDetailStat;
        alwaysOnline = uc.alwaysOnline;
        tariffName   = uc.tariffName;
        address      = uc.address;
        phone        = uc.phone;
        email        = uc.email;
        note         = uc.note;
        realName     = uc.realName;
        corp         = uc.corp;
        group        = uc.group;
        credit       = uc.credit;
        nextTariff   = uc.nextTariff;
        for (size_t i = 0; i < USERDATA_NUM; i++) userdata[i]  = uc.userdata[i];
        services     = uc.services;
        creditExpire = uc.creditExpire;
        ips          = uc.ips;
        return *this;
    }
    //-------------------------------------------------------------------------

    UserConfOpt(const UserConfOpt&) = default;
    UserConfOpt& operator=(const UserConfOpt&) = default;
    UserConfOpt(UserConfOpt&&) = default;
    UserConfOpt& operator=(UserConfOpt&&) = default;

    std::optional<std::string>               password;
    std::optional<int>                       passive;
    std::optional<int>                       disabled;
    std::optional<int>                       disabledDetailStat;
    std::optional<int>                       alwaysOnline;
    std::optional<std::string>               tariffName;
    std::optional<std::string>               address;
    std::optional<std::string>               phone;
    std::optional<std::string>               email;
    std::optional<std::string>               note;
    std::optional<std::string>               realName;
    std::optional<std::string>               corp;
    std::optional<std::string>               group;
    std::optional<double>                    credit;
    std::optional<std::string>               nextTariff;
    std::vector<std::optional<std::string> > userdata;
    std::optional<std::vector<std::string> > services;
    std::optional<time_t>                    creditExpire;
    std::optional<UserIPs>                   ips;
};
//-----------------------------------------------------------------------------
}
