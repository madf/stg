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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

 /*
 $Revision: 1.8 $
 $Author: faust $
 $Date: 2009/10/22 09:58:53 $
 */

#include <fstream>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "stg/common.h"
#include "nrmap_parser.h"

bool NRMapParser::ReadFile(const std::string & fileName)
{
std::ifstream source(fileName.c_str());

if (!source)
    {
    errorStr = "Error opening file ";
    errorStr += fileName;
    printfd(__FILE__, "NRMapParser::ReadFile(): %s\n", errorStr.c_str());
    return true;
    }

int lineNumber = 0;
std::string line;
std::vector<NET_ROUTER> _nrmap;

while (getline(source, line))
    {
    ++lineNumber;
    NET_ROUTER nr;

    if (Trim(line) == "")
        {
        continue;
        }

    if (ParseLine(line, nr))
        {
        printfd(__FILE__, "NRMapParser::ReadFile(): Error parsing line %d: '%s'\n", lineNumber, errorStr.c_str());
        return true;
        }

    _nrmap.push_back(nr);
    }

nrmap = _nrmap;

return false;
}

bool NRMapParser::ParseLine(const std::string & line, NET_ROUTER & nr) const
{
// xxx.xxx.xxx.xxx/yy zzz.zzz.zzz.zzz
size_t pos = line.find_first_of(" \t");

if (pos == std::string::npos)
    {
    errorStr = "No space between subnet and router";
    return true;
    }

std::string subnet(line.substr(0, pos)); // xxx.xxx.xxx.xxx/yy

uint32_t ip = 0;
uint32_t mask = 0;

if (ParseNet(subnet, ip, mask))
    {
    return true;
    }

nr.subnetIP = ip;
nr.subnetMask = mask;

pos = line.find_first_not_of(" \t", pos);

if (pos == std::string::npos)
    {
    errorStr = "No router address found";
    return true;
    }

size_t pos2 = line.find_first_of(" \t", pos);

std::string router(line.substr(pos, pos2 == std::string::npos ? line.length() - pos2 - 1 : pos2 - pos)); //zzz.zzz.zzz.zzz

uint32_t routerIP;

if (ParseRouter(router, routerIP))
    {
    return true;
    }

std::vector<uint32_t>::iterator it;

it = std::lower_bound(
        nr.routers.begin(),
        nr.routers.end(),
        routerIP
        );
nr.routers.insert(it, routerIP);

//nr.routers.push_back(routerIP);

while (pos2 != std::string::npos)
    {
    pos = line.find_first_not_of(" \t", pos2);

    if (pos == std::string::npos)
        {
        return false;
        }

    pos2 = line.find_first_of(" \t", pos);

    if (ParseRouter(line.substr(
                        pos,
                        pos2 == std::string::npos ? line.length() - pos2 - 1 : pos2 - pos),
                    routerIP))
        {
        return true;
        }

    it = std::lower_bound(
            nr.routers.begin(),
            nr.routers.end(),
            routerIP
            );
    nr.routers.insert(it, routerIP);

    //nr.routers.push_back(routerIP);

    }

return false;
}

bool NRMapParser::ParseNet(const std::string & line, uint32_t & ip, uint32_t & mask) const
{
// xxx.xxx.xxx.xxx/yy

size_t pos = line.find_first_of('/');

if (pos == std::string::npos)
    {
    errorStr = "Subnet is not in CIDR notation";
    return true;
    }

int res = inet_pton(AF_INET, line.substr(0, pos).c_str(), &ip); //xxx.xxx.xxx.xxx

if (res < 0)
    {
    errorStr = strerror(errno);
    return true;
    }
else if (res == 0)
    {
    errorStr = "Invalid subnet address";
    return true;
    }

if (str2x(line.substr(pos + 1, line.length() - pos - 1), mask)) //yy
    {
    errorStr = "Invalid subnet mask";
    return true;
    }
if (mask > 32)
    {
    errorStr = "Subnet mask is out of range [0..32]";
    return true;
    }
mask = htonl(0xffFFffFF << (32 - mask)); //bitmask

return false;
}

bool NRMapParser::ParseRouter(const std::string & line, uint32_t & ip) const
{
int res = inet_pton(AF_INET, line.c_str(), &ip); //zzz.zzz.zzz.zzz

if (res < 0)
    {
    errorStr = strerror(errno);
    return true;
    }
else if (res == 0)
    {
    printfd(__FILE__, "NRMapParser::ParseRouter(): IP '%s' is invalid\n", line.c_str());
    errorStr = "Invalid router address";
    return true;
    }
return false;
}
