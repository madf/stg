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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#pragma once

#define RS_MAGIC_LEN        (6)
#define RS_PROTO_VER_LEN    (2)
#define RS_MAX_PACKET_LEN   (1048)
#define RS_LOGIN_LEN        (32)
#define RS_PARAMS_LEN       (979)

#define RS_ALIVE_PACKET      (0)
#define RS_CONNECT_PACKET    (1)
#define RS_DISCONNECT_PACKET (2)

#define RS_ID "RSP00"

#include <cstdint>

namespace STG::RS
{

struct PACKET_HEADER
{
int8_t              magic[RS_MAGIC_LEN];
int8_t              protoVer[RS_PROTO_VER_LEN];
int8_t              packetType;
uint32_t            ip;
uint32_t            id;
int8_t              login[RS_LOGIN_LEN];
int8_t              padding[7];
} __attribute__((__packed__)); // 48 bytes, 6 blocks

struct PACKET_TAIL
{
int8_t              magic[RS_MAGIC_LEN];
int8_t              params[RS_PARAMS_LEN];
int8_t              padding[7];
} __attribute__((__packed__)); // 992 bytes, 124 blocks

} // namespace RS
