#ifndef __STG_SGCP_PACKET_H__
#define __STG_SGCP_PACKET_H__

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

#include <boost/stdint.hpp>

namespace STG
{
namespace SGCP
{

struct __attribute__ ((__packed__)) Packet
{
    Packet(uint16_t type, uint16_t size);

    bool valid() const { return magic == MAGIC; }

    static uint64_t MAGIC;
    static uint16_t VERSION;

    enum Types { PING, PONG, DATA };

    uint64_t magic;
    uint64_t senderTime;
    uint16_t version;
    uint16_t type;
    uint32_t size;
};

Packet hton(Packet value);

} // namespace SGCP
} // namespace STG

#endif
