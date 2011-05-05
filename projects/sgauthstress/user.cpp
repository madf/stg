#include "user.h"

USER::USER(const std::string & l,
           const std::string & pwd)
    : login(l),
      phase(1),
      rnd(0),
      sock(0)
{
char key[IA_PASSWD_LEN];
memset(key, 0, IA_PASSWD_LEN);
strncpy(key, password.c_str(), IA_PASSWD_LEN);
Blowfish_Init(&ctx, key, IA_PASSWD_LEN);
}

USER::~USER()
{
}

void USER::Connect()
{
}

void USER::Disconnect()
{
}
