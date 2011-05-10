#include <sys/socket.h>
#include <netinet/in.h>

#include <cstring>

#include "user.h"
#include "stg/ia_packets.h"

USER::USER(const std::string & l,
           const std::string & pwd,
           uint32_t i)
    : login(l),
      password(pwd),
      ip(i),
      aliveTimeout(0),
      userTimeout(0),
      phase(1),
      phaseChangeTime(0),
      rnd(0)
{
unsigned char key[IA_PASSWD_LEN];
memset(key, 0, IA_PASSWD_LEN);
strncpy((char *)key, password.c_str(), IA_PASSWD_LEN);
Blowfish_Init(&ctx, key, IA_PASSWD_LEN);

sock = socket(AF_INET, SOCK_DGRAM, 0);
}

USER::~USER()
{
close(sock);
}
