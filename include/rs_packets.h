#ifndef RS_PACKETSH
#define RS_PACKETSH

#define RS_MAGIC_LEN        (6)
#define RS_PROTO_VER_LEN    (2)
#define RS_MAX_PACKET_LEN   (1048)
#define RS_LOGIN_LEN        (32)
#define RS_PARAMS_LEN       (979)

#define RS_ALIVE_PACKET      (0)
#define RS_CONNECT_PACKET    (1)
#define RS_DISCONNECT_PACKET (2)

#define RS_ID "RSP00"

#include "os_int.h"

struct RS_PACKET_HEADER
{
int8_t              magic[RS_MAGIC_LEN];
int8_t              protoVer[RS_PROTO_VER_LEN];
int8_t              packetType;
uint32_t            ip;
uint32_t            id;
int8_t              login[RS_LOGIN_LEN];
int8_t              padding[7];
} __attribute__((__packed__)); // 48 bytes, 6 blocks

struct RS_PACKET_TAIL
{
int8_t              magic[RS_MAGIC_LEN];
int8_t              params[RS_PARAMS_LEN];
int8_t              padding[7];
} __attribute__((__packed__)); // 992 bytes, 124 blocks

#endif
