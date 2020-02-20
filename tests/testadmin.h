#ifndef __TEST_ADMIN_H__
#define __TEST_ADMIN_H__

#include "stg/admin.h"

class TEST_ADMIN : public STG::Admin {
    public:
        TEST_ADMIN() : priv(0xffFF), ip(0) {}

        const std::string & GetPassword() const override { return password; }
        const std::string & GetLogin() const override { return login; }
        STG::Priv const *        GetPriv() const override { return &priv; }
        uint32_t            GetPrivAsInt() const override { return priv.toInt(); }
        const STG::AdminConf &  GetConf() const override { return conf; }
        uint32_t            GetIP() const override { return ip; }
        std::string         GetIPStr() const override { return inet_ntostring(ip); }
        void                SetIP(uint32_t ip) override { TEST_ADMIN::ip = ip; }
        const std::string   GetLogStr() const override { return ""; }

    private:
        std::string password;
        std::string login;
        STG::Priv priv;
        STG::AdminConf conf;
        uint32_t ip;
};

#endif
