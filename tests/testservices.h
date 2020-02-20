#ifndef __TEST_SERVICES__
#define __TEST_SERVICES__

#include "stg/services.h"

class TEST_SERVICES : public STG::Services
{
    public:
        virtual int Add(const STG::ServiceConf & /*service*/, const STG::Admin * /*admin*/) { return 0; }
        virtual int Del(const std::string & /*name*/, const STG::Admin * /*admin*/) { return 0; }
        virtual int Change(const STG::ServiceConf & /*service*/, const STG::Admin * /*admin*/) { return 0; }
        virtual bool Find(const std::string & /*name*/, STG::ServiceConf * /*service*/) const { return false; }
        virtual bool Find(const std::string & /*name*/, STG::ServiceConfOpt * /*service*/) const { return false; }
        virtual bool Exists(const std::string & /*name*/) const { return false; }
        virtual const std::string & GetStrError() const { return m_errorStr; }
        virtual size_t Count() const { return 0; }

        virtual int OpenSearch() const { return 0; }
        virtual int SearchNext(int, STG::ServiceConf * /*service*/) const { return 0; }
        virtual int CloseSearch(int) const { return 0; }

    private:
        std::string m_errorStr;
};

#endif
