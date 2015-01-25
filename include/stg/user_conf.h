 /*
 $Revision: 1.12 $
 $Date: 2010/03/11 14:42:05 $
 $Author: faust $
 */

#ifndef USER_CONF_H
#define USER_CONF_H

#include <string>
#include <vector>
#include "const.h"
#include "user_ips.h"
#include "resetable.h"
#include "os_int.h"

//-----------------------------------------------------------------------------
struct USER_CONF
{
    USER_CONF()
        : passive(0),
          disabled(0),
          disabledDetailStat(0),
          alwaysOnline(0),
          credit(0),
          userdata(USERDATA_NUM),
          creditExpire(0)
    {}

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
    std::vector<std::string> service;
    std::string              group;
    double                   credit;
    std::string              nextTariff;
    std::vector<std::string> userdata;
    time_t                   creditExpire;
    USER_IPS                 ips;
};
//-----------------------------------------------------------------------------
struct USER_CONF_RES
{
    USER_CONF_RES()
        : userdata(USERDATA_NUM)
    {}
    USER_CONF_RES & operator=(const USER_CONF & uc)
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
        group        = uc.group;
        credit       = uc.credit;
        nextTariff   = uc.nextTariff;
        for (int i = 0; i < USERDATA_NUM; i++)
            {
            userdata[i]  = uc.userdata[i];
            }
        creditExpire = uc.creditExpire;
        ips          = uc.ips;
        return *this;
    }
    USER_CONF GetData() const
    {
        USER_CONF uc;
        uc.password     = password.data();
        uc.passive      = passive.data();
        uc.disabled     = disabled.data();
        uc.disabledDetailStat = disabledDetailStat.data();
        uc.alwaysOnline = alwaysOnline.data();
        uc.tariffName   = tariffName.data();
        uc.address      = address.data();
        uc.phone        = phone.data();
        uc.email        = email.data();
        uc.note         = note.data();
        uc.realName     = realName.data();
        uc.group        = group.data();
        uc.credit       = credit.data();
        uc.nextTariff   = nextTariff.data();
        for (int i = 0; i < USERDATA_NUM; i++)
            {
            uc.userdata[i]  = userdata[i].data();
            }
        uc.creditExpire = creditExpire.data();
        uc.ips          = ips.data();
        return uc;
    }
    //-------------------------------------------------------------------------

    RESETABLE<std::string>               password;
    RESETABLE<int>                       passive;
    RESETABLE<int>                       disabled;
    RESETABLE<int>                       disabledDetailStat;
    RESETABLE<int>                       alwaysOnline;
    RESETABLE<std::string>               tariffName;
    RESETABLE<std::string>               address;
    RESETABLE<std::string>               phone;
    RESETABLE<std::string>               email;
    RESETABLE<std::string>               note;
    RESETABLE<std::string>               realName;
    RESETABLE<std::string>               group;
    RESETABLE<double>                    credit;
    RESETABLE<std::string>               nextTariff;
    std::vector<RESETABLE<std::string> > userdata;
    RESETABLE<time_t>                    creditExpire;
    RESETABLE<USER_IPS>                  ips;
};
//-----------------------------------------------------------------------------
#endif
