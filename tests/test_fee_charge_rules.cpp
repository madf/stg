#include "tut/tut.hpp"

#include "stg/settings.h"
#include "stg/user_property.h"
#include "user_impl.h"
#include "tariff_impl.h"

const volatile time_t stgTime = 0;

namespace tut
{
    struct fee_charge_rules_data {
    };

    typedef test_group<fee_charge_rules_data> tg;
    tg fee_charge_rules_test_group("Fee charge rules tests group");

    typedef tg::object testobject;

    class TEST_SETTINGS : public SETTINGS {
        public:
            TEST_SETTINGS(unsigned _feeChargeType)
                : feeChargeType(_feeChargeType) {}

            const std::string & GetDirName(size_t) const { return dirName; }
            const std::string & GetScriptsDir() const { return scriptsDir; }
            unsigned            GetDetailStatWritePeriod() const { return 10; }
            unsigned            GetStatWritePeriod() const { return 10; }
            unsigned            GetDayFee() const { return 0; }
            bool                GetFullFee() const { return false; }
            unsigned            GetDayResetTraff() const { return 0; }
            bool                GetSpreadFee() const { return false; }
            bool                GetFreeMbAllowInet() const { return false; }
            bool                GetDayFeeIsLastDay() const { return false; }
            bool                GetWriteFreeMbTraffCost() const { return false; }
            bool                GetShowFeeInCash() const { return false; }
            unsigned            GetMessageTimeout() const { return 0; }
            unsigned            GetFeeChargeType() const { return feeChargeType; }
            const std::string & GetMonitorDir() const { return monitorDir; }
            bool                GetMonitoring() const { return false; }

        private:
            std::string dirName;
            std::string scriptsDir;
            std::string monitorDir;
            unsigned feeChargeType;
    };

    class TEST_TARIFFS : public TARIFFS {
        public:
            TEST_TARIFFS() {}

            int            ReadTariffs () { return 0; }
            const TARIFF * FindByName(const std::string & name) const { return &testTariff; }
            const TARIFF * GetNoTariff() const { return NULL; }
            int            GetTariffsNum() const { return 0; }
            int            Del(const std::string & name, const ADMIN * admin) { return 0; }
            int            Add(const std::string & name, const ADMIN * admin) { return 0; }
            int            Chg(const TARIFF_DATA & td, const ADMIN * admin) { return 0; }

            void           GetTariffsData(std::list<TARIFF_DATA> * tdl) {}

            const std::string & GetStrError() const { return strError; }

            void           SetFee(double fee);

        private:
            std::string strError;
            TARIFF_IMPL testTariff;
    };

    class TEST_ADMIN : public ADMIN {
        public:
            TEST_ADMIN() : priv(0xffFFffFF) {}

            ADMIN & operator=(const ADMIN &) { return *this; }
            ADMIN & operator=(const ADMIN_CONF &) { return *this; }
            bool    operator==(const ADMIN & rhs) const { return true; }
            bool    operator!=(const ADMIN & rhs) const { return false; }
            bool    operator<(const ADMIN & rhs) const { return true; }
            bool    operator<=(const ADMIN & rhs) const { return true; }

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

    class TEST_STORE : public STORE {
        public:
            TEST_STORE() {}

            int GetUsersList(std::vector<std::string> * usersList) const { return 0; }
            int AddUser(const std::string & login) const { return 0; }
            int DelUser(const std::string & login) const { return 0; }
            int SaveUserStat(const USER_STAT & stat, const std::string & login) const { return 0; }
            int SaveUserConf(const USER_CONF & conf, const std::string & login) const { return 0; }
            int RestoreUserStat(USER_STAT * stat, const std::string & login) const { return 0; }
            int RestoreUserConf(USER_CONF * conf, const std::string & login) const { return 0; }

            int WriteUserChgLog(const std::string & login,
                                const std::string & admLogin,
                                uint32_t admIP,
                                const std::string & paramName,
                                const std::string & oldValue,
                                const std::string & newValue,
                                const std::string & message = "") const { return 0; }

            int WriteUserConnect(const std::string & login, uint32_t ip) const { return 0; }

            int WriteUserDisconnect(const std::string & login,
                                    const DIR_TRAFF & up,
                                    const DIR_TRAFF & down,
                                    const DIR_TRAFF & sessionUp,
                                    const DIR_TRAFF & sessionDown,
                                    double cash,
                                    double freeMb,
                                    const std::string & reason) const { return 0; }

            int WriteDetailedStat(const TRAFF_STAT & statTree,
                                  time_t lastStat,
                                  const std::string & login) const { return 0; }

            int AddMessage(STG_MSG * msg, const std::string & login) const { return 0; }
            int EditMessage(const STG_MSG & msg, const std::string & login) const { return 0; }
            int GetMessage(uint64_t id, STG_MSG * msg, const std::string & login) const { return 0; }
            int DelMessage(uint64_t id, const std::string & login) const { return 0; }
            int GetMessageHdrs(vector<STG_MSG_HDR> * hdrsList, const std::string & login) const { return 0; }

            int SaveMonthStat(const USER_STAT & stat, int month, int year, const std::string & login) const { return 0; }

            int GetAdminsList(std::vector<std::string> * adminsList) const { return 0; }
            int SaveAdmin(const ADMIN_CONF & ac) const { return 0; }
            int RestoreAdmin(ADMIN_CONF * ac, const std::string & login) const { return 0; }
            int AddAdmin(const std::string & login) const { return 0; }
            int DelAdmin(const std::string & login) const { return 0; }

            int GetTariffsList(std::vector<std::string> * tariffsList) const { return 0; }
            int AddTariff(const std::string & name) const { return 0; }
            int DelTariff(const std::string & name) const { return 0; }
            int SaveTariff(const TARIFF_DATA & td, const std::string & tariffName) const { return 0; }
            int RestoreTariff(TARIFF_DATA * td, const std::string & tariffName) const { return 0; }

            int GetCorpsList(std::vector<std::string> * corpsList) const { return 0; }
            int SaveCorp(const CORP_CONF & cc) const { return 0; }
            int RestoreCorp(CORP_CONF * cc, const std::string & name) const { return 0; }
            int AddCorp(const std::string & name) const { return 0; }
            int DelCorp(const std::string & name) const { return 0; }

            int GetServicesList(std::vector<std::string> * corpsList) const { return 0; }
            int SaveService(const SERVICE_CONF & sc) const { return 0; }
            int RestoreService(SERVICE_CONF * sc, const std::string & name) const { return 0; }
            int AddService(const std::string & name) const { return 0; }
            int DelService(const std::string & name) const { return 0; }

            void SetSettings(const MODULE_SETTINGS & s) {}
            int ParseSettings() { return 0; }
            const std::string & GetStrError() const { return strError; }
            const std::string & GetVersion() const { return version; }

        private:
            std::string strError;
            std::string version;
    };

    template<>
    template<>
    void testobject::test<1>()
    {
        set_test_name("Check classic rules");

        TEST_SETTINGS settings(0);
        TEST_TARIFFS tariffs;
        TEST_ADMIN admin;
        TEST_STORE store;
        USER_IMPL user(&settings, &store, &tariffs, &admin, NULL);

        USER_PROPERTY<double> & cash(user.GetProperty().cash);
        USER_PROPERTY<std::string> & tariffName(user.GetProperty().tariffName);

        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
        cash = 100;
        ensure_equals("user.cash == 100", user.GetProperty().cash, 100);

        tariffs.SetFee(50);
        tariffName = "test";
        ensure_equals("user.tariffName == 'test'", user.GetProperty().tariffName.ConstData(), "test");
        user.ProcessDayFee();
        ensure_equals("user.cash == 50", user.GetProperty().cash, 50);
        user.ProcessDayFee();
        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
        user.ProcessDayFee();
        ensure_equals("user.cash == -50", user.GetProperty().cash, -50);
    }

    template<>
    template<>
    void testobject::test<2>()
    {
        set_test_name("Check second rules (allow fee if cash value is positive)");

        TEST_SETTINGS settings(1);
        TEST_TARIFFS tariffs;
        TEST_ADMIN admin;
        TEST_STORE store;
        USER_IMPL user(&settings, &store, &tariffs, &admin, NULL);

        USER_PROPERTY<double> & cash(user.GetProperty().cash);
        USER_PROPERTY<std::string> & tariffName(user.GetProperty().tariffName);

        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
        cash = 100;
        ensure_equals("user.cash == 100", user.GetProperty().cash, 100);

        tariffs.SetFee(50);
        tariffName = "test";
        ensure_equals("user.tariffName == 'test'", user.GetProperty().tariffName.ConstData(), "test");
        user.ProcessDayFee();
        ensure_equals("user.cash == 50", user.GetProperty().cash, 50);
        user.ProcessDayFee();
        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
        user.ProcessDayFee();
        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
        cash = 49;
        ensure_equals("user.cash == 49", user.GetProperty().cash, 49);
        user.ProcessDayFee();
        ensure_equals("user.cash == -1", user.GetProperty().cash, -1);
    }

    template<>
    template<>
    void testobject::test<3>()
    {
        set_test_name("Check third rules (allow fee if cash value is greater than fee)");

        TEST_SETTINGS settings(2);
        TEST_TARIFFS tariffs;
        TEST_ADMIN admin;
        TEST_STORE store;
        USER_IMPL user(&settings, &store, &tariffs, &admin, NULL);

        USER_PROPERTY<double> & cash(user.GetProperty().cash);
        USER_PROPERTY<std::string> & tariffName(user.GetProperty().tariffName);

        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
        cash = 100;
        ensure_equals("user.cash == 100", user.GetProperty().cash, 100);

        tariffs.SetFee(50);
        tariffName = "test";
        ensure_equals("user.tariffName == 'test'", user.GetProperty().tariffName.ConstData(), "test");
        user.ProcessDayFee();
        ensure_equals("user.cash == 50", user.GetProperty().cash, 50);
        user.ProcessDayFee();
        ensure_equals("user.cash == 50", user.GetProperty().cash, 50);
        tariffs.SetFee(49);
        user.ProcessDayFee();
        ensure_equals("user.cash == 1", user.GetProperty().cash, 1);
        cash = 0;
        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
        user.ProcessDayFee();
        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
    }

    void TEST_TARIFFS::SetFee(double fee)
    {
        TARIFF_DATA td(testTariff.GetTariffData());
        td.tariffConf.fee = fee;
        testTariff = td;
    }
}
