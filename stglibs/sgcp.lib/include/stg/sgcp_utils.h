#ifndef __STG_SGCP_UTILS_H__
#define __STG_SGCP_UTILS_H__

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

#include "stg/os_int.h"

#include <arpa/inet.h> // hton*

namespace STG
{
namespace SGCP
{

template <typename T> inline T hton(T value) { return value; }
template <typename T> inline T ntoh(T value) { return hton(value); }

template <> inline uint16_t hton(uint16_t value) { return htons(value); }
template <> inline int16_t hton(int16_t value) { return htons(value); }

template <> inline uint32_t hton(uint32_t value) { return htonl(value); }
template <> inline int32_t hton(int32_t value) { return htonl(value); }

inline
uint64_t htonll(uint64_t value)
{
#ifdef ARCH_LE
    const uint32_t high_part = htonl(static_cast<uint32_t>(value >> 32));
    const uint32_t low_part = htonl(static_cast<uint32_t>(value & 0xFFFFFFFFLL));

    return (static_cast<uint64_t>(low_part) << 32) | high_part;
#else
    return value;
#endif
}

template <> inline uint64_t hton(uint64_t value) { return htonll(value); }
template <> inline int64_t hton(int64_t value) { return htonll(value); }

} // namespace SGCP
} // namespace STG

#endif
