 /*
 $Revision: 1.1 $
 $Date: 2010/09/10 01:45:24 $
 $Author: faust $
 */

#ifndef ADMIN_CONF_INC_H
#define ADMIN_CONF_INC_H

inline
uint32_t PRIV::ToInt() const
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

inline
void PRIV::FromInt(uint32_t p)
{
userStat   = (p & 0x00000003) >> 0x00; // 1+2
userConf   = (p & 0x0000000C) >> 0x02; // 4+8
userCash   = (p & 0x00000030) >> 0x04; // 10+20
userPasswd = (p & 0x000000C0) >> 0x06; // 40+80
userAddDel = (p & 0x00000300) >> 0x08; // 100+200
adminChg   = (p & 0x00000C00) >> 0x0A; // 400+800
tariffChg  = (p & 0x00003000) >> 0x0C; // 1000+2000
serviceChg = (p & 0x0000C000) >> 0x0E; // 4000+8000
corpChg    = (p & 0x00030000) >> 0x10; // 10000+20000
}

#endif
