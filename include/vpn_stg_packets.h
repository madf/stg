#ifndef vpn_stg_packets_h
#define vpn_stg_packets_h

#define VS_MAGIC "VS01"

enum 
{
GET_LOGIN,
GET_LOGIN_ANS,
VS_ALIVE,
VS_ALIVE_ANS,
VS_DISCONNECT
}


struct VS_GET_LOGIN
{
char magic[4];
STG_PACKET_TYPES type;
char login[32];
//char password[32];
};

struct VS_CHECK_LOGIN_ANS
{
char magic[4];
STG_PACKET_TYPES type;
char login[32];
char password[32];
uint32_t ip;
};

struct VS_ALIVE
{
char magic[4];
STG_PACKET_TYPES type;
char login[32];
uint32_t ip;
};

struct VS_ALIVE_ANS
{
char magic[4];
STG_PACKET_TYPES type;
char login[32];
uint32_t ip;
};

struct VS_DISCONNECT
{
char magic[4];
STG_PACKET_TYPES type;
char login[32];
uint32_t ip;
};


#endif
