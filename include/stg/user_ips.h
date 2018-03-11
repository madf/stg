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
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

 /*
 $Revision: 1.22 $
 $Date: 2010/03/04 11:49:53 $
 $Author: faust $
 */

#ifndef USER_IPS_H
#define USER_IPS_H

#include "stg/common.h"
#include "os_int.h"

#include <cstring>
#include <vector>
#include <string>
#include <iostream>

#ifdef FREE_BSD
#include <sys/types.h>
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//-------------------------------------------------------------------------
struct IP_MASK
{
IP_MASK() : ip(0), mask(0) {}
uint32_t ip;
uint32_t mask;
};
//-------------------------------------------------------------------------
class USER_IPS
{
    friend std::ostream & operator<< (std::ostream & o, const USER_IPS & i);
    friend const USER_IPS StrToIPS(const std::string & ipsStr);

public:
    typedef std::vector<IP_MASK> ContainerType;
    typedef ContainerType::size_type IndexType;

    const IP_MASK & operator[](IndexType idx) const { return ips[idx]; }
    std::string GetIpStr() const;
    bool IsIPInIPS(uint32_t ip) const;
    bool OnlyOneIP() const;
    bool IsAnyIP() const;
    size_t Count() const { return ips.size(); }
    void Add(const IP_MASK &im) { ips.push_back(im); }

private:
    uint32_t CalcMask(unsigned int msk) const;
    ContainerType ips;
};
//-------------------------------------------------------------------------

inline
std::string USER_IPS::GetIpStr() const
{
if (ips.empty())
    return "";

if (ips[0].ip == 0)
    return "*";

ContainerType::const_iterator it(ips.begin());
std::string res = inet_ntostring(it->ip);
++it;
for (; it != ips.end(); ++it)
    res += "," + inet_ntostring(it->ip);
return res;
}
//-----------------------------------------------------------------------------
inline
uint32_t USER_IPS::CalcMask(unsigned int msk) const
{
if (msk > 32)
    return 0;
return htonl(0xFFffFFff << (32 - msk));
}
//-----------------------------------------------------------------------------
inline
bool USER_IPS::IsIPInIPS(uint32_t ip) const
{
if (ips.empty())
    return false;

if (ips.front().ip == 0)
    return true;

for (ContainerType::const_iterator it(ips.begin()); it != ips.end(); ++it)
    {
    uint32_t mask(CalcMask(it->mask));
    if ((ip & mask) == (it->ip & mask))
        return true;
    }
return false;
}
//-----------------------------------------------------------------------------
inline
bool USER_IPS::OnlyOneIP() const
{
if (ips.size() == 1 && ips.front().mask == 32 && ips.front().ip != 0)
    return true;

return false;
}
//-----------------------------------------------------------------------------
inline
bool USER_IPS::IsAnyIP() const
{
    return !ips.empty() && ips.front().ip == 0;
}
//-----------------------------------------------------------------------------
inline
std::ostream & operator<<(std::ostream & o, const USER_IPS & i)
{
return o << i.GetIpStr();
}
//-----------------------------------------------------------------------------
inline
const USER_IPS StrToIPS(const std::string & ipsStr)
{
USER_IPS ips;
std::vector<std::string> ipMask;
if (ipsStr.empty())
    return ips;

if (ipsStr[0] == '*' && ipsStr.size() == 1)
    {
    ips.ips.push_back(IP_MASK());
    return ips;
    }

char * tmp = new char[ipsStr.size() + 1];
strcpy(tmp, ipsStr.c_str());
char * pstr = tmp;
char * paddr = NULL;
while ((paddr = strtok(pstr, ",")))
    {
    pstr = NULL;
    ipMask.push_back(paddr);
    }

delete[] tmp;

for (USER_IPS::IndexType i = 0; i < ipMask.size(); i++)
    {
    char str[128];
    char * strIp;
    char * strMask;
    strcpy(str, ipMask[i].c_str());
    strIp = strtok(str, "/");
    if (strIp == NULL)
        return ips;
    strMask = strtok(NULL, "/");

    IP_MASK im;

    im.ip = inet_addr(strIp);
    if (im.ip == INADDR_NONE)
        return ips;

    im.mask = 32;
    if (strMask != NULL)
        {
        int m = 0;
        if (str2x(strMask, m) != 0)
            return ips;
        im.mask = m;

        if (im.mask > 32)
            return ips;

        if ((im.ip & ips.CalcMask(im.mask)) != im.ip)
            return ips;
        }
    ips.ips.push_back(im);
    }

return ips;
}
//-------------------------------------------------------------------------
#endif //USER_IPS_H
