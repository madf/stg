#pragma once

#include "stg/tariffs.h"

#include "tariff_impl.h"

class TestTariffs : public STG::Tariffs
{
    public:
        TestTariffs() : m_tariff("") {}

        int            ReadTariffs() override { return 0; }
        const STG::Tariff* FindByName(const std::string& /*name*/) const override { return &m_tariff; }
        const STG::Tariff* GetNoTariff() const override { return NULL; }
        int            Del(const std::string& /*name*/, const STG::Admin* /*admin*/) override { return 0; }
        int            Add(const std::string& /*name*/, const STG::Admin* /*admin*/) override { return 0; }
        int            Chg(const STG::TariffData& /*td*/, const STG::Admin* /*admin*/) override { return 0; }

        void AddNotifierAdd(STG::NotifierBase<STG::TariffData>*) override {}
        void DelNotifierAdd(STG::NotifierBase<STG::TariffData>*) override {}

        void AddNotifierDel(STG::NotifierBase<STG::TariffData>*) override {}
        void DelNotifierDel(STG::NotifierBase<STG::TariffData>*) override {}

        void           GetTariffsData(std::vector<STG::TariffData>* /*tdl*/) const override {}

        size_t         Count() const override { return 0; }

        const std::string& GetStrError() const override { return m_errorStr; }

        void           SetFee(double fee)
        {
            STG::TariffData td(m_tariff.GetTariffData());
            td.tariffConf.fee = fee;
            m_tariff = td;
        }

    private:
        std::string m_errorStr;
        STG::TariffImpl m_tariff;
};
