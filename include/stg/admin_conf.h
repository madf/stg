 /*
 $Revision: 1.9 $
 $Date: 2010/09/10 05:02:08 $
 $Author: faust $
 */

#ifndef ADMIN_CONF_H
#define ADMIN_CONF_H

#include "stg/resetable.h"

#include <string>

#include "os_int.h"

#define ADM_LOGIN_LEN   (32)
#define ADM_PASSWD_LEN  (32)
//-----------------------------------------------------------------------------
struct PRIV
{
    PRIV()
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
    PRIV(uint32_t p)
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

    uint32_t ToInt() const;
    void FromInt(uint32_t p);

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
struct ADMIN_CONF
{
    ADMIN_CONF()
        : priv(),
          login(),
          password("* NO PASSWORD *")
    {}
    ADMIN_CONF(const ADMIN_CONF & rvalue)
        : priv(rvalue.priv),
          login(rvalue.login),
          password(rvalue.password)
    {}
    ADMIN_CONF(const PRIV & pr, const std::string & l, const std::string & p)
        : priv(pr),
          login(l),
          password(p)
    {}
    PRIV          priv;
    std::string   login;
    std::string   password;
};
//-----------------------------------------------------------------------------
struct ADMIN_CONF_RES
{
    ADMIN_CONF_RES() {}
    ADMIN_CONF_RES(const ADMIN_CONF_RES & rhs)
        : priv(rhs.priv),
          login(rhs.login),
          password(rhs.password)
    {}
    ADMIN_CONF_RES & operator=(const ADMIN_CONF_RES & rhs)
    {
        priv = rhs.priv;
        login = rhs.login;
        password = rhs.password;
        return *this;
    }
    RESETABLE<PRIV> priv;
    RESETABLE<std::string> login;
    RESETABLE<std::string> password;
};

#include "admin_conf.inc.h"

#endif


