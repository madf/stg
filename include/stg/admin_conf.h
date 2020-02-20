#pragma once

#include "stg/optional.h"

#include <string>

#include <cstdint>

#define ADM_LOGIN_LEN   (32)
#define ADM_PASSWD_LEN  (32)

namespace STG
{

struct Priv
{
    Priv() noexcept
        : userStat(0),
          userConf(0),
          userCash(0),
          userPasswd(0),
          userAddDel(0),
          adminChg(0),
          tariffChg(0),
          serviceChg(0),
          corpChg(0)
    {}
    explicit Priv(uint32_t p) noexcept
        : userStat((p & 0x00000003) >> 0x00),
          userConf((p & 0x0000000C) >> 0x02),
          userCash((p & 0x00000030) >> 0x04),
          userPasswd((p & 0x000000C0) >> 0x06),
          userAddDel((p & 0x00000300) >> 0x08),
          adminChg((p & 0x00000C00) >> 0x0A),
          tariffChg((p & 0x00003000) >> 0x0C),
          serviceChg((p & 0x0000C000) >> 0x0E),
          corpChg((p & 0x00030000) >> 0x10)
    {}

    Priv(const Priv&) = default;
    Priv& operator=(const Priv&) = default;
    Priv(Priv&&) = default;
    Priv& operator=(Priv&&) = default;

    uint32_t toInt() const noexcept
    {
        uint32_t p = (userStat   << 0)  |
                     (userConf   << 2)  |
                     (userCash   << 4)  |
                     (userPasswd << 6)  |
                     (userAddDel << 8)  |
                     (adminChg   << 10) |
                     (tariffChg  << 12) |
                     (serviceChg << 14) |
                     (corpChg    << 16);
        return p;
    }

    uint16_t userStat;
    uint16_t userConf;
    uint16_t userCash;
    uint16_t userPasswd;
    uint16_t userAddDel;
    uint16_t adminChg;
    uint16_t tariffChg;
    uint16_t serviceChg;
    uint16_t corpChg;
};
//-----------------------------------------------------------------------------
struct AdminConf
{
    AdminConf()
        : password("* NO PASSWORD *")
    {}
    AdminConf(const Priv & pr, const std::string & l, const std::string & p)
        : priv(pr),
          login(l),
          password(p)
    {}

    AdminConf(const AdminConf&) = default;
    AdminConf& operator=(const AdminConf&) = default;
    AdminConf(AdminConf&&) = default;
    AdminConf& operator=(AdminConf&&) = default;

    Priv          priv;
    std::string   login;
    std::string   password;
};
//-----------------------------------------------------------------------------
struct AdminConfOpt
{
    Optional<Priv> priv;
    Optional<std::string> login;
    Optional<std::string> password;
};

}
