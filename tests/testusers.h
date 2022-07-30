#ifndef __TEST_USERS_H__
#define __TEST_USERS_H__

#include "stg/users.h"

class TEST_USERS : public STG::Users {
    public:
        TEST_USERS() {}

        using UserPtr = STG::User*;
        using ConstUserPtr = const STG::User*;

        int  FindByName(const std::string & /*login*/, UserPtr * /*user*/) override
        { return -1; }
        int  FindByName(const std::string & /*login*/, ConstUserPtr * /*user*/) const override
        { return -1; }

        bool TariffInUse(const std::string & /*tariffName*/) const override
        { return -1; }

        void AddNotifierUserAdd(STG::NotifierBase<UserPtr> * /*notifier*/) override {}
        void DelNotifierUserAdd(STG::NotifierBase<UserPtr> * /*notifier*/) override {}

        void AddNotifierUserDel(STG::NotifierBase<UserPtr> * /*notifier*/) override {}
        void DelNotifierUserDel(STG::NotifierBase<UserPtr> * /*notifier*/) override {}

        int  Add(const std::string & /*login*/, const STG::Admin * /*admin*/) override
        { return 0; }
        void Del(const std::string & /*login*/, const STG::Admin * /*admin*/) override {}

        bool Authorize(const std::string &, uint32_t, uint32_t, const STG::Auth *) override
        { return false; }
        bool Unauthorize(const std::string &, const STG::Auth *, const std::string &) override
        { return false; }

        int  ReadUsers() override { return 0; }
        virtual size_t Count() const override { return 0; };

        int  FindByIPIdx(uint32_t /*ip*/, UserPtr * /*user*/) const override
        { return -1; }
        bool IsIPInIndex(uint32_t /*ip*/) const override { return false; }
        bool IsIPInUse(uint32_t, const std::string &, ConstUserPtr *) const override { return false; }
        bool Exists(const std::string &) const override { return false; }

        unsigned int  OpenSearch() override { return 0; }
        int  SearchNext(int /*handle*/, UserPtr * /*u*/) override { return -1; }
        int  CloseSearch(int /*handle*/) override { return 0; }

        int  Start() override { return 0; }
        int  Stop() override { return 0; }

    private:
};

#endif
