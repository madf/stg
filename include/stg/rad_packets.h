#ifndef RAD_PACKETSH
#define RAD_PACKETSH

#define RAD_MAGIC_LEN        (5)
#define RAD_PROTO_VER_LEN    (2)
#define RAD_MAX_PACKET_LEN   (1024)
#define RAD_LOGIN_LEN        (32)
#define RAD_SERVICE_LEN        (16)
#define RAD_PASSWORD_LEN        (32)
#define RAD_SESSID_LEN        (32)

// Request
#define RAD_AUTZ_PACKET      (0)
#define RAD_AUTH_PACKET      (1)
#define RAD_POST_AUTH_PACKET (2)
#define RAD_ACCT_START_PACKET (3)
#define RAD_ACCT_STOP_PACKET (4)
#define RAD_ACCT_UPDATE_PACKET (5)
#define RAD_ACCT_OTHER_PACKET (6)
// Responce
#define RAD_ACCEPT_PACKET    (7)
#define RAD_REJECT_PACKET    (8)

#define RAD_ID "00100"

#include <cstdint>

struct RAD_PACKET
{
uint8_t              magic[RAD_MAGIC_LEN];
uint8_t              protoVer[RAD_PROTO_VER_LEN];
uint8_t              packetType;
uint8_t              login[RAD_LOGIN_LEN];
uint32_t             ip;
uint8_t              service[RAD_SERVICE_LEN];
uint8_t              password[RAD_PASSWORD_LEN];
uint8_t              sessid[RAD_SESSID_LEN];
uint8_t              padding[4];
};

#endif
