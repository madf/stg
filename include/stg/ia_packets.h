#ifndef PACKETH
#define PACKETH

#include <cstdint>

#define CONN_SYN_N          0
#define CONN_SYN_ACK_N      1
#define CONN_ACK_N          2
#define ALIVE_SYN_N         3
#define ALIVE_ACK_N         4
#define DISCONN_SYN_N       5
#define DISCONN_SYN_ACK_N   6
#define DISCONN_ACK_N       7
#define FIN_N               8
#define ERROR_N             9
#define INFO_N              10
#define INFO_7_N            11
#define INFO_8_N            12
#define UPDATE_N            13

#define DIR_NUM             (10)

#define IA_FREEMB_LEN       (16)
#define IA_LOGIN_LEN        (32)
#define IA_PASSWD_LEN       (32)
#define IA_MAX_TYPE_LEN     (16)
#define IA_MAX_MSG_LEN      (235)
#define IA_MAX_MSG_LEN_8    (1030)
#define IA_DIR_NAME_LEN     (16)
#define IA_MAGIC_LEN        (6)
#define IA_PROTO_VER_LEN    (2)

#define ST_NOT_INETABLE     (0)
#define ST_INETABLE         (1)

#define IA_ID "00100"

typedef int8_t string16[IA_DIR_NAME_LEN];
//-----------------------------------------------------------------------------
struct HDR_8
{
int8_t          magic[IA_MAGIC_LEN];
int8_t          protoVer[IA_PROTO_VER_LEN];
//uint32_t        ip;
//int8_t          padding[4];
};
//-----------------------------------------------------------------------------
struct CONN_SYN_6
{
int8_t          magic[IA_MAGIC_LEN];
int8_t          protoVer[IA_PROTO_VER_LEN];
int8_t          loginS[IA_LOGIN_LEN];
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
int8_t          login[IA_LOGIN_LEN];
int8_t          padding[2];
};
//-----------------------------------------------------------------------------
struct CONN_SYN_8
{
HDR_8           hdr;
int8_t          loginS[IA_LOGIN_LEN];
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
int8_t          login[IA_LOGIN_LEN];
uint32_t        dirs;   // Byte-order dependent
};
//-----------------------------------------------------------------------------
struct CONN_SYN_ACK_6
{
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
uint32_t        rnd;    // Byte-order dependent
int32_t         userTimeOut;    // Byte-order dependent
int32_t         aliveDelay;     // Byte-order dependent
string16        dirName[DIR_NUM];
};
//-----------------------------------------------------------------------------
struct CONN_SYN_ACK_8
{
HDR_8           hdr;
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
uint32_t        rnd;    // Byte-order dependent
int32_t         userTimeOut;    // Byte-order dependent
int32_t         aliveDelay;     // Byte-order dependent
string16        dirName[DIR_NUM];
};
//-----------------------------------------------------------------------------
struct CONN_ACK_6
{
int8_t          magic[IA_MAGIC_LEN];
int8_t          protoVer[IA_PROTO_VER_LEN];
int8_t          loginS[IA_LOGIN_LEN];
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
uint32_t        rnd;    // Byte-order dependent
};
//-----------------------------------------------------------------------------
struct CONN_ACK_8
{
HDR_8           hdr;
int8_t          loginS[IA_LOGIN_LEN];
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
uint32_t        rnd;    // Byte-order dependent
};
//-----------------------------------------------------------------------------
struct ALIVE_SYN_6
{
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
uint32_t        rnd;    // Byte-order dependent

int64_t         mu[DIR_NUM];    // Byte-order dependent
int64_t         md[DIR_NUM];    // Byte-order dependent

int64_t         su[DIR_NUM];    // Byte-order dependent
int64_t         sd[DIR_NUM];    // Byte-order dependent

int64_t         cash;           // Byte-order dependent

int8_t          freeMb[IA_FREEMB_LEN];
};
//-----------------------------------------------------------------------------
struct ALIVE_SYN_8
{
HDR_8           hdr;
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
uint32_t        rnd;    // Byte-order dependent

int64_t         mu[DIR_NUM];    // Byte-order dependent
int64_t         md[DIR_NUM];    // Byte-order dependent

int64_t         su[DIR_NUM];    // Byte-order dependent
int64_t         sd[DIR_NUM];    // Byte-order dependent

int64_t         cash; // Деньги умноженные на 1000 - Byte-order dependent
int8_t          freeMb[IA_FREEMB_LEN];

uint32_t        status; // Byte-order dependent
int8_t          padding[4];
};
//-----------------------------------------------------------------------------
struct ALIVE_ACK_6
{
int8_t          magic[IA_MAGIC_LEN];
int8_t          protoVer[IA_PROTO_VER_LEN];
int8_t          loginS[IA_LOGIN_LEN];
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
uint32_t        rnd;    // Byte-order dependent
};
//-----------------------------------------------------------------------------
struct ALIVE_ACK_8
{
HDR_8           hdr;
int8_t          loginS[IA_LOGIN_LEN];
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
uint32_t        rnd;    // Byte-order dependent
};
//-----------------------------------------------------------------------------
struct DISCONN_SYN_6
{
int8_t          magic[IA_MAGIC_LEN];
int8_t          protoVer[IA_PROTO_VER_LEN];
int8_t          loginS[IA_LOGIN_LEN];
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
int8_t          login[IA_LOGIN_LEN];
int8_t          padding[2];
};
//-----------------------------------------------------------------------------
struct DISCONN_SYN_8
{
HDR_8           hdr;
int8_t          loginS[IA_LOGIN_LEN];
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
int8_t          login[IA_LOGIN_LEN];
int8_t          padding[4];
};
//-----------------------------------------------------------------------------
struct DISCONN_SYN_ACK_6
{
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
uint32_t        rnd;    // Byte-order dependent
};
//-----------------------------------------------------------------------------
struct DISCONN_SYN_ACK_8
{
HDR_8           hdr;
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
uint32_t        rnd;    // Byte-order dependent
};
//-----------------------------------------------------------------------------
struct DISCONN_ACK_6
{
int8_t          magic[IA_MAGIC_LEN];
int8_t          protoVer[IA_PROTO_VER_LEN];
int8_t          loginS[IA_LOGIN_LEN];
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
uint32_t        rnd;    // Byte-order dependent
};
//-----------------------------------------------------------------------------
struct DISCONN_ACK_8
{
HDR_8           hdr;
int8_t          loginS[IA_LOGIN_LEN];
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
uint32_t        rnd;    // Byte-order dependent
};
//-----------------------------------------------------------------------------
struct FIN_6
{
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
int8_t          ok[3];
int8_t          padding[1];
};
//-----------------------------------------------------------------------------
struct FIN_8
{
HDR_8           hdr;
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
int8_t          ok[3];
int8_t          padding[1];
};
//-----------------------------------------------------------------------------
struct ERR
{
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
int8_t          text[236];
};
//-----------------------------------------------------------------------------
struct ERR_8
{
HDR_8           hdr;
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
int8_t          text[236];
};
//-----------------------------------------------------------------------------
struct INFO_6
{
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
int8_t          infoType;
int8_t          text[IA_MAX_MSG_LEN];
};
//-----------------------------------------------------------------------------
struct INFO_7
{
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
int8_t          infoType;
uint32_t        sendTime;   // Byte-order dependent
int8_t          showTime;
int8_t          text[IA_MAX_MSG_LEN];
int8_t          padding[5];
};
//-----------------------------------------------------------------------------
struct INFO_8
{
HDR_8           hdr;
int32_t         len;    // Byte-order dependent
int8_t          type[IA_MAX_TYPE_LEN];
int8_t          infoType;
uint32_t        sendTime;   // Byte-order dependent
int8_t          showTime;
int8_t          text[IA_MAX_MSG_LEN_8];
};
//-----------------------------------------------------------------------------
struct LOADSTAT
{
int64_t         mu[DIR_NUM];    // Byte-order dependent
int64_t         md[DIR_NUM];    // Byte-order dependent

int64_t         su[DIR_NUM];    // Byte-order dependent
int64_t         sd[DIR_NUM];    // Byte-order dependent

int64_t         cash;   // Деньги умноженные на 1000 - Byte-order dependent
int8_t          freeMb[IA_FREEMB_LEN];
int32_t         status; // Byte-order dependent
};
//-----------------------------------------------------------------------------
#define CONN_SYN_7          CONN_SYN_6
#define CONN_SYN_ACK_7      CONN_SYN_ACK_6
#define CONN_ACK_7          CONN_ACK_6
#define ALIVE_SYN_7         ALIVE_SYN_6
#define ALIVE_ACK_7         ALIVE_ACK_6
#define DISCONN_SYN_7       DISCONN_SYN_6
#define DISCONN_SYN_ACK_7   DISCONN_SYN_ACK_6
#define DISCONN_ACK_7       DISCONN_ACK_6
#define FIN_7               FIN_6

#endif
