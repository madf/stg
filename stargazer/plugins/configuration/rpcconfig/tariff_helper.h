#pragma once

#include <xmlrpc-c/base.hpp>

namespace STG
{

struct TariffData;

}

class TARIFF_HELPER
{
public:
    explicit TARIFF_HELPER(STG::TariffData & td)
        : data(td)
    {}

    void GetTariffInfo(xmlrpc_c::value * info) const;
    bool SetTariffInfo(const xmlrpc_c::value & info);
private:
    STG::TariffData & data;
};
