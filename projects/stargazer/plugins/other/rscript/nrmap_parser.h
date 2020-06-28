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
 *    Author : Maksym Mamontov <stg@madf.info>
 */

 /*
 $Revision: 1.2 $
 $Author: faust $
 $Date: 2009/09/23 12:51:42 $
 */

#ifndef __NRMAP_PARSER_H__
#define __NRMAP_PARSER_H__

#include <string>
#include <vector>

#include "stg/os_int.h"

struct NET_ROUTER
{
    NET_ROUTER() : subnetIP(0), subnetMask(0), routers() {}
    NET_ROUTER(const NET_ROUTER & rvalue)
        : subnetIP(rvalue.subnetIP),
          subnetMask(rvalue.subnetMask),
          routers(rvalue.routers)
    {}

    uint32_t              subnetIP;
    uint32_t              subnetMask;
    std::vector<uint32_t> routers;

    NET_ROUTER & operator=(const NET_ROUTER & rvalue)
    {
    subnetIP = rvalue.subnetIP;
    subnetMask = rvalue.subnetMask;
    routers = rvalue.routers;
    return *this;
    }
};

class NRMapParser {
public:
    NRMapParser() : nrmap(), errorStr() {}
    ~NRMapParser() {}

    bool ReadFile(const std::string & fileName);
    const std::vector<NET_ROUTER> & GetMap() const { return nrmap; }
    const std::string & GetErrorStr() const { return errorStr; }

private:
    NRMapParser(const NRMapParser & rvalue);
    NRMapParser & operator=(const NRMapParser & rvalue);

    std::vector<NET_ROUTER> nrmap;
    mutable std::string errorStr;

    bool ParseLine(const std::string & line, NET_ROUTER & nr) const;
    bool ParseNet(const std::string & line, uint32_t & ip, uint32_t & mask) const;
    bool ParseRouter(const std::string & line, uint32_t & ip) const;
};

#endif
