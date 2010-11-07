/*
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *    Date: 27.10.2002
 */

/*
 *    Author : Boris Mikhailenko <stg34@ua.fm>
 */


#ifndef HOSTALLOW_H
#define HOSTALLOW_H

#include "os_int.h"

#include <list>

using namespace std;

#define HA_ERR_MSG_LEN  (255)
//-----------------------------------------------------------------------------
enum ORDER 
{
    orderAllow = 0,
    orderDeny
};
//-----------------------------------------------------------------------------
enum 
{
    hostsAllow = 0,
    hostsDeny
};
//-----------------------------------------------------------------------------
struct INETADDR
{
    INETADDR(uint32_t i, uint8_t m) {ip = i; mask = m;};
    INETADDR() {ip = 0; mask = 0;};
    uint32_t ip;
    uint8_t  mask;
};
//-----------------------------------------------------------------------------
class HOSTALLOW
{
public:
    HOSTALLOW();
    int ParseHosts(const char *, int hostsType);
    int ParseOrder(const char *);
    int GetError();
    bool HostAllowed(uint32_t ip);
    const char * GetStrError();

private:
    int ParseIPMask(const char * s, uint32_t * ip, uint32_t * mask);
    bool IsHostInDeniedList(uint32_t ip);
    bool IsHostInAllowedList(uint32_t ip);
    //int  IsIPInSubnet(INETADDR &ia);
    int  IsIPInSubnet(uint32_t checkedIP, INETADDR &ia);
    list<INETADDR> hostAllowList;
    list<INETADDR> hostDenyList;
    char errMsg[HA_ERR_MSG_LEN];
    int order;
    char src[16 + 1];
    char dst[16 + 1];

};
//-----------------------------------------------------------------------------

#endif
