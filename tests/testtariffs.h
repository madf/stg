#ifndef __TEST_TARIFFS_H__
#define __TEST_TARIFFS_H__

#include "stg/tariffs.h"

#include "tariff_impl.h"

class TEST_TARIFFS : public STG::Tariffs {
    public:
        TEST_TARIFFS() : testTariff("") {}

        int            ReadTariffs() override { return 0; }
        const STG::Tariff * FindByName(const std::string & /*name*/) const override { return &testTariff; }
        const STG::Tariff * GetNoTariff() const override { return NULL; }
        int            Del(const std::string & /*name*/, const STG::Admin * /*admin*/) override { return 0; }
        int            Add(const std::string & /*name*/, const STG::Admin * /*admin*/) override { return 0; }
        int            Chg(const STG::TariffData & /*td*/, const STG::Admin * /*admin*/) override { return 0; }

        void AddNotifierAdd(STG::NotifierBase<STG::TariffData> *) override {}
        void DelNotifierAdd(STG::NotifierBase<STG::TariffData> *) override {}

        void AddNotifierDel(STG::NotifierBase<STG::TariffData> *) override {}
        void DelNotifierDel(STG::NotifierBase<STG::TariffData> *) override {}

        void           GetTariffsData(std::vector<STG::TariffData> * /*tdl*/) const override {}

        size_t         Count() const override { return 0; }

        const std::string & GetStrError() const override { return strError; }

        void           SetFee(double fee);

    private:
        std::string strError;
        STG::TariffImpl testTariff;
};

inline
void TEST_TARIFFS::SetFee(double fee)
{
    STG::TariffData td(testTariff.GetTariffData());
    td.tariffConf.fee = fee;
    testTariff = td;
}

#endif
