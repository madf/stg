 /*
 $Revision: 1.1 $
 $Date: 2010/09/10 01:45:24 $
 $Author: faust $
 */

#ifndef ADMIN_CONF_INC_H
#define ADMIN_CONF_INC_H

inline
uint16_t PRIV::ToInt() const
{
uint16_t p = (userStat   << 0)  |
             (userConf   << 2)  |
             (userCash   << 4)  |
             (userPasswd << 6)  |
             (userAddDel << 8)  |
             (adminChg   << 10) |
             (tariffChg  << 12);
return p;
}

inline
void PRIV::FromInt(uint16_t p)
{
userStat   = (p & 0x0003) >> 0x00; // 1+2
userConf   = (p & 0x000C) >> 0x02; // 4+8
userCash   = (p & 0x0030) >> 0x04; // 10+20
userPasswd = (p & 0x00C0) >> 0x06; // 40+80
userAddDel = (p & 0x0300) >> 0x08; // 100+200
adminChg   = (p & 0x0C00) >> 0x0A; // 400+800
tariffChg  = (p & 0x3000) >> 0x0C; // 1000+2000
}

#endif
