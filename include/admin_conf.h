 /*
 $Revision: 1.9 $
 $Date: 2010/09/10 05:02:08 $
 $Author: faust $
 */

#ifndef ADMIN_CONF_H
#define ADMIN_CONF_H

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
          tariffChg(0)
    {};
    PRIV(uint16_t p)
        : userStat((p & 0x0003) >> 0x00),
          userConf((p & 0x000C) >> 0x02),
          userCash((p & 0x0030) >> 0x04),
          userPasswd((p & 0x00C0) >> 0x06),
          userAddDel((p & 0x0300) >> 0x08),
          adminChg((p & 0x0C00) >> 0x0A),
          tariffChg((p & 0x3000) >> 0x0C)
    {}

    uint16_t ToInt() const;
    void FromInt(uint16_t p);

    uint16_t userStat;
    uint16_t userConf;
    uint16_t userCash;
    uint16_t userPasswd;
    uint16_t userAddDel;
    uint16_t adminChg;
    uint16_t tariffChg;
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

#include "admin_conf.inc.h"

#endif


