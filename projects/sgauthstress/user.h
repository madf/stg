#ifndef __USER_H__
#define __USER_H__

#include <string>

#include "stg/os_int.h"
#include "stg/ia.h"

class USER {
    public:
        USER(const std::string & login,
             const std::string & password);
        ~USER();

        void Connect();
        void Disconnect();

    private:
        const std::string login;
        int phase;
        int rnd;
        int sock;
        BLOWFISH_CTX ctx;
};

#endif
