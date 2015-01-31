#ifndef __TEST_SERVICES__
#define __TEST_SERVICES__

#include "stg/services.h"

class TEST_SERVICES : public SERVICES
{
    public:
        virtual int Add(const SERVICE_CONF & /*service*/, const ADMIN * /*admin*/) { return 0; }
        virtual int Del(const std::string & /*name*/, const ADMIN * /*admin*/) { return 0; }
        virtual int Change(const SERVICE_CONF & /*service*/, const ADMIN * /*admin*/) { return 0; }
        virtual bool Find(const std::string & name, SERVICE_CONF * service) const { return false; }
        virtual bool Find(const std::string & name, SERVICE_CONF_RES * service) const { return false; }
        virtual bool Exists(const std::string & name) const { return false; }
        virtual const std::string & GetStrError() const { return m_errorStr; }
        virtual size_t Count() const { return 0; }

        virtual int OpenSearch() const { return 0; }
        virtual int SearchNext(int, SERVICE_CONF * /*service*/) const { return 0; }
        virtual int CloseSearch(int) const { return 0; }

    private:
        std::string m_errorStr;
};

#endif
