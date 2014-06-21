#ifndef __TEST_USERS_H__
#define __TEST_USERS_H__

class TEST_USERS : public USERS {
    public:
        TEST_USERS() {}

        int  FindByName(const std::string & /*login*/, USER_PTR * /*user*/)
        { return -1; }
        int  FindByName(const std::string & /*login*/, CONST_USER_PTR * /*user*/) const
        { return -1; }

        bool TariffInUse(const std::string & /*tariffName*/) const
        { return -1; }

        void AddNotifierUserAdd(NOTIFIER_BASE<USER_PTR> * /*notifier*/) {}
        void DelNotifierUserAdd(NOTIFIER_BASE<USER_PTR> * /*notifier*/) {}

        void AddNotifierUserDel(NOTIFIER_BASE<USER_PTR> * /*notifier*/) {}
        void DelNotifierUserDel(NOTIFIER_BASE<USER_PTR> * /*notifier*/) {}

        int  Add(const std::string & /*login*/, const ADMIN * /*admin*/)
        { return 0; }
        void Del(const std::string & /*login*/, const ADMIN * /*admin*/) {}

        bool Authorize(const std::string &, uint32_t, uint32_t, const AUTH *)
        { return false; }
        bool Unauthorize(const std::string &, const AUTH *, const std::string &)
        { return false; }

        int  ReadUsers() { return 0; }
        virtual size_t Count() const { return 0; };

        int  FindByIPIdx(uint32_t /*ip*/, USER_PTR * /*user*/) const
        { return -1; }
        bool IsIPInIndex(uint32_t /*ip*/) const { return false; }
        bool IsIPInUse(uint32_t, const std::string &, CONST_USER_PTR *) const { return false; }

        int  OpenSearch() { return 0; }
        int  SearchNext(int /*handle*/, USER_PTR * /*u*/) { return -1; }
        int  CloseSearch(int /*handle*/) { return 0; }

        int  Start() { return 0; }
        int  Stop() { return 0; }

    private:
};

#endif
