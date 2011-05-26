#ifndef __TEST_ADMIN_H__
#define __TEST_ADMIN_H__

#include "stg/admin.h"

class TEST_ADMIN : public ADMIN {
    public:
        TEST_ADMIN() : priv(0xffFF) {}

        ADMIN & operator=(const ADMIN &) { return *this; }
        ADMIN & operator=(const ADMIN_CONF &) { return *this; }
        bool    operator==(const ADMIN & /*rhs*/) const { return true; }
        bool    operator!=(const ADMIN & /*rhs*/) const { return false; }
        bool    operator<(const ADMIN & /*rhs*/) const { return true; }
        bool    operator<=(const ADMIN & /*rhs*/) const { return true; }

        const std::string & GetPassword() const { return password; }
        const std::string & GetLogin() const { return login; }
        PRIV const *        GetPriv() const { return &priv; }
        uint16_t            GetPrivAsInt() const { return priv.ToInt(); }
        const ADMIN_CONF &  GetConf() const { return conf; }
        uint32_t            GetIP() const { return ip; }
        std::string         GetIPStr() const { return inet_ntostring(ip); }
        void                SetIP(uint32_t ip) { TEST_ADMIN::ip = ip; }
        const std::string   GetLogStr() const { return ""; }
    
    private:
        std::string password;
        std::string login;
        PRIV priv;
        ADMIN_CONF conf;
        uint32_t ip;
};

#endif
