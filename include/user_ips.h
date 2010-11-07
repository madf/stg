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

#include <vector>
//#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
#include <cstring>
/////////////////////////
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"
#include "os_int.h"

using namespace std;

//-------------------------------------------------------------------------
struct IP_MASK
{
IP_MASK() : ip(0), mask(0) {}
IP_MASK(const IP_MASK & ipm) : ip(ipm.ip), mask(ipm.mask)  {}
uint32_t ip;
uint32_t mask;
};
//-------------------------------------------------------------------------
class USER_IPS
{
    friend std::ostream & operator<< (ostream & o, const USER_IPS & i);
    //friend stringstream & operator<< (stringstream & s, const USER_IPS & i);
    friend const USER_IPS StrToIPS(const string & ipsStr) throw(string);

public:
    USER_IPS();
    USER_IPS(const USER_IPS &);
    USER_IPS & operator=(const USER_IPS &);
    const IP_MASK & operator[](int idx) const;
    std::string GetIpStr() const;
    bool IsIPInIPS(uint32_t ip) const;
    bool OnlyOneIP() const;
    int  Count() const;
    void Add(const IP_MASK &im);
    void Erase();

private:
    uint32_t CalcMask(unsigned int msk) const;
    std::vector<IP_MASK> ips;
};
//-------------------------------------------------------------------------

//-----------------------------------------------------------------------------
inline
USER_IPS::USER_IPS()
    : ips()
{}
//-----------------------------------------------------------------------------
inline
USER_IPS::USER_IPS(const USER_IPS & i)
    : ips(i.ips)
{}
//-----------------------------------------------------------------------------
inline
USER_IPS & USER_IPS::operator=(const USER_IPS & i)
{
ips = i.ips;
return *this;
}
//-----------------------------------------------------------------------------
inline
const IP_MASK & USER_IPS::operator[](int idx) const
{
return ips[idx];
}
//-----------------------------------------------------------------------------
inline
std::string USER_IPS::GetIpStr() const
{
if (ips.empty())
    {
    return "";
    }

if (ips[0].ip == 0)
    {
    return "*";
    }

std::vector<IP_MASK>::const_iterator it(ips.begin());
std::stringstream s;
s << inet_ntostring(it->ip);
++it;
for (; it != ips.end(); ++it)
    {
    s << "," << inet_ntostring(it->ip);
    }
return s.str();
}
//-----------------------------------------------------------------------------
inline
int USER_IPS::Count() const
{
return ips.size();
}
//-----------------------------------------------------------------------------
inline
bool USER_IPS::IsIPInIPS(uint32_t ip) const
{
if (ips.empty())
    {
    return false;
    }

if (ips.front().ip == 0)
    return true;

for (std::vector<IP_MASK>::const_iterator it(ips.begin()); it != ips.end(); ++it)
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
if (ips.size() == 1 && ips.front().mask == 32)
    return true;

return false;
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
void USER_IPS::Add(const IP_MASK &im)
{
ips.push_back(im);
}
//-----------------------------------------------------------------------------
inline
void USER_IPS::Erase()
{
ips.erase(ips.begin(), ips.end());
}
//-----------------------------------------------------------------------------
inline
std::ostream & operator<<(std::ostream & o, const USER_IPS & i)
{
return o << i.GetIpStr();
}
//-----------------------------------------------------------------------------
/*inline
stringstream & operator<<(std::stringstream & s, const USER_IPS & i)
{
s << i.GetIpStr();
return s;
}*/
//-----------------------------------------------------------------------------
inline
const USER_IPS StrToIPS(const std::string & ipsStr) throw(std::string)
{
USER_IPS ips;
char * paddr;
IP_MASK im;
std::vector<std::string> ipMask;
std::string err;
if (ipsStr.empty())
    {
    err = "Incorrect IP address.";
    throw(err);
    }

if (ipsStr[0] == '*' && ipsStr.size() == 1)
    {
    im.ip = 0;
    im.mask = 0;
    ips.ips.push_back(im);
    return ips;
    }

char * str = new char[ipsStr.size() + 1];
strcpy(str, ipsStr.c_str());
char * pstr = str;
while ((paddr = strtok(pstr, ",")))
    {
    pstr = NULL;
    ipMask.push_back(paddr);
    }

delete[] str;

for (unsigned int i = 0; i < ipMask.size(); i++)
    {
    char str[128];
    char * strIp;
    char * strMask;
    strcpy(str, ipMask[i].c_str());
    strIp = strtok(str, "/");
    if (strIp == NULL)
        {
        err = "Incorrect IP address " + ipsStr;
        throw(err);
        }
    strMask = strtok(NULL, "/");

    im.ip = inet_addr(strIp);
    if (im.ip == INADDR_NONE)
        {
        err = "Incorrect IP address: " + std::string(strIp);
        throw(err);
        }

    im.mask = 32;
    if (strMask != NULL)
        {
        int m = 0;
        if (str2x(strMask, m) != 0)
            {
            err = "Incorrect mask: " + std::string(strMask);
            throw(err);
            }
        im.mask = m;

        if (im.mask > 32)
            {
            err = "Incorrect mask: " + std::string(strMask);
            throw(err);
            }

        if ((im.ip & ips.CalcMask(im.mask)) != im.ip)
            {
            err = "Address does'n match mask: " + std::string(strIp) + "/" + std::string(strMask);
            throw(err);
            }
        }
    ips.ips.push_back(im);
    }

return ips;
}
//-------------------------------------------------------------------------
#endif //USER_IPS_H


