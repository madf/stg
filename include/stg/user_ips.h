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

#pragma once

#include "stg/common.h"

#include <vector>
#include <string>
#include <ostream>
#include <cstring>
#include <cstdint>

#ifdef FREE_BSD
#include <sys/types.h>
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace STG
{

//-------------------------------------------------------------------------
struct IPMask
{
    IPMask() noexcept : ip(0), mask(0) {}
    IPMask(uint32_t i, uint32_t m) noexcept : ip(i), mask(m) {}

    IPMask(const IPMask&) = default;
    IPMask& operator=(const IPMask&) = default;
    IPMask(IPMask&&) = default;
    IPMask& operator=(IPMask&&) = default;

    uint32_t ip;
    uint32_t mask;
};
//-------------------------------------------------------------------------
class UserIPs
{
    friend std::ostream & operator<< (std::ostream & o, const UserIPs & i);

    public:
        using ContainerType = std::vector<IPMask>;
        using IndexType = ContainerType::size_type;

        UserIPs() = default;

        UserIPs(const UserIPs&) = default;
        UserIPs& operator=(const UserIPs&) = default;
        UserIPs(UserIPs&&) = default;
        UserIPs& operator=(UserIPs&&) = default;

        static UserIPs parse(const std::string& source);

        const IPMask& operator[](IndexType idx) const noexcept { return ips[idx]; }
        std::string toString() const noexcept;
        bool find(uint32_t ip) const noexcept;
        bool onlyOneIP() const noexcept;
        bool isAnyIP() const noexcept;
        size_t count() const noexcept { return ips.size(); }
        void add(const IPMask& im)  noexcept{ ips.push_back(im); }

    private:
        uint32_t calcMask(unsigned int msk) const noexcept;
        ContainerType ips;
};
//-------------------------------------------------------------------------

inline
std::string UserIPs::toString() const noexcept
{
    if (ips.empty())
        return "";

    if (ips[0].ip == 0)
        return "*";

    auto it = ips.begin();
    std::string res = inet_ntostring(it->ip);
    ++it;
    for (; it != ips.end(); ++it)
        res += "," + inet_ntostring(it->ip);
    return res;
}
//-----------------------------------------------------------------------------
inline
uint32_t UserIPs::calcMask(unsigned int msk) const noexcept
{
    if (msk > 32)
        return 0;
    return htonl(0xFFffFFff << (32 - msk));
}
//-----------------------------------------------------------------------------
inline
bool UserIPs::find(uint32_t ip) const noexcept
{
    if (ips.empty())
        return false;

    if (ips.front().ip == 0)
        return true;

    for (auto it = ips.begin(); it != ips.end(); ++it)
    {
        const auto mask = calcMask(it->mask);
        if ((ip & mask) == (it->ip & mask))
            return true;
    }
    return false;
}
//-----------------------------------------------------------------------------
inline
bool UserIPs::onlyOneIP() const noexcept
{
    if (ips.size() == 1 && ips.front().mask == 32 && ips.front().ip != 0)
        return true;

    return false;
}
//-----------------------------------------------------------------------------
inline
bool UserIPs::isAnyIP() const noexcept
{
    return !ips.empty() && ips.front().ip == 0;
}
//-----------------------------------------------------------------------------
inline
std::ostream & operator<<(std::ostream& o, const UserIPs& i)
{
    return o << i.toString();
}
//-----------------------------------------------------------------------------
inline
UserIPs UserIPs::parse(const std::string& source)
{
    if (source.empty())
        return {};

    UserIPs ips;
    if (source[0] == '*' && source.size() == 1)
    {
        ips.ips.push_back(IPMask());
        return ips;
    }

    std::vector<std::string> ipMask;
    char * tmp = new char[source.size() + 1];
    strcpy(tmp, source.c_str());
    char * pstr = tmp;
    char * paddr = NULL;
    while ((paddr = strtok(pstr, ",")))
    {
        pstr = NULL;
        ipMask.push_back(paddr);
    }

    delete[] tmp;

    for (UserIPs::IndexType i = 0; i < ipMask.size(); i++)
    {
        char str[128];
        char * strIp;
        char * strMask;
        strcpy(str, ipMask[i].c_str());
        strIp = strtok(str, "/");
        if (strIp == NULL)
            return ips;
        strMask = strtok(NULL, "/");

        IPMask im;

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

            if ((im.ip & ips.calcMask(im.mask)) != im.ip)
                return ips;
        }
        ips.ips.push_back(im);
    }

    return ips;
}
//-------------------------------------------------------------------------
}
