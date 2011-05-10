#ifndef __USER_H__
#define __USER_H__

#include <ctime>
#include <string>

#include "stg/os_int.h"

class USER {
    public:
        USER(const std::string & login,
             const std::string & password,
             uint32_t ip);
        ~USER();

        const std::string & GetLogin() const { return login; }
        uint32_t GetIP() const { return ip; }
        uint32_t GetAliveTimeout() const { return aliveTimeout; }
        uint32_t GetUserTimeout() const { return userTimeout; }
        int GetPhase() const { return phase; }
        int GetRnd() const { return rnd; }
        int GetSocket() const { return sock; }
        time_t GetPhaseChangeTime() const { return phaseChangeTime; }

        BLOWFISH_CTX * GetCtx() { return &ctx; }

        void SetPhase(int p) { phase = p; time(&phaseChangeTime); }
        void SetRnd(int r) { rnd = r; }
        int IncRnd() { return ++rnd; }
        void SetAliveTimeout(uint32_t timeout) { aliveTimeout = timeout; }
        void SetUserTimeout(uint32_t timeout) { userTimeout = timeout; }

    private:
        std::string login;
        std::string password;
        uint32_t ip;
        uint32_t aliveTimeout;
        uint32_t userTimeout;
        int phase;
        time_t phaseChangeTime;
        int rnd;
        int sock;
        BLOWFISH_CTX ctx;
};

#endif
