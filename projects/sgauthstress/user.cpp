#include <sys/socket.h>
#include <netinet/in.h>

#include <cstring>
#include <cerrno>
#include <stdexcept>

#include "user.h"
#include "stg/ia_packets.h"
#include "stg/common.h"

USER::USER(const std::string & l,
           const std::string & pwd,
           uint32_t i)
    : login(l),
      password(pwd),
      ip(i),
      aliveTimeout(5),
      userTimeout(60),
      phase(1),
      phaseChangeTime(0),
      rnd(0),
      sock(-1)
{
unsigned char key[IA_PASSWD_LEN];
memset(key, 0, IA_PASSWD_LEN);
strncpy((char *)key, password.c_str(), IA_PASSWD_LEN);
Blowfish_Init(&ctx, key, IA_PASSWD_LEN);
}

USER::USER(const USER & rvalue)
    : login(rvalue.login),
      password(rvalue.password),
      ip(rvalue.ip),
      aliveTimeout(rvalue.aliveTimeout),
      userTimeout(rvalue.userTimeout),
      phase(1),
      phaseChangeTime(0),
      rnd(0),
      sock(-1)
{
unsigned char key[IA_PASSWD_LEN];
memset(key, 0, IA_PASSWD_LEN);
strncpy((char *)key, password.c_str(), IA_PASSWD_LEN);
Blowfish_Init(&ctx, key, IA_PASSWD_LEN);
}

USER::~USER()
{
if (sock > 0)
    close(sock);
}

USER & USER::operator=(const USER & rvalue)
{
login = rvalue.login;
password = rvalue.password;
ip = rvalue.ip;
aliveTimeout = rvalue.aliveTimeout;
userTimeout = rvalue.userTimeout;
phase = 1;
phaseChangeTime = 0;
rnd = 0;
sock = -1;

unsigned char key[IA_PASSWD_LEN];
memset(key, 0, IA_PASSWD_LEN);
strncpy((char *)key, password.c_str(), IA_PASSWD_LEN);
Blowfish_Init(&ctx, key, IA_PASSWD_LEN);

return *this;
}

bool USER::InitNetwork()
{
sock = socket(AF_INET, SOCK_DGRAM, 0);

if (sock < 0)
    {
    throw std::runtime_error(std::string("USER::USER() - socket creation error: '") + strerror(errno) + "', ip: " + inet_ntostring(ip) + ", login: " + login);
    }

struct sockaddr_in addr;

addr.sin_family = AF_INET;
addr.sin_addr.s_addr = ip;
addr.sin_port = htons(5554); // :(

int res = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
if (res == -1)
    {
    throw std::runtime_error(std::string("USER::USER() - bind error: '") + strerror(errno) + "', ip: " + inet_ntostring(ip) + ", login: " + login);
    }

return true;
}
