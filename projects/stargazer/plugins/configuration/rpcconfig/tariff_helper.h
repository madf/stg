#ifndef __TARIFF_HELPER_H__
#define __TARIFF_HELPER_H__

#include <xmlrpc-c/base.hpp>
#include "stg/tariff_conf.h"

class TARIFF_HELPER
{
public:
    TARIFF_HELPER(TARIFF_DATA & td)
        : data(td)
    {}

    void GetTariffInfo(xmlrpc_c::value * info) const;
    bool SetTariffInfo(const xmlrpc_c::value & info);
private:
    TARIFF_DATA & data;
};

#endif


