#pragma once

#include "sensors.h"

#include <string>
#include <map>

namespace STG
{
class Tariffs;
class Users;

class TableSensor {
    public:
        explicit TableSensor(const std::string & p) : prefix(p) {}
        virtual ~TableSensor() {}

        const std::string & GetPrefix() const { return prefix; }
        virtual void UpdateSensors(Sensors & sensors) const = 0;

    protected:
        std::string prefix;
};

class TariffUsersTable : public TableSensor {
    public:
        TariffUsersTable(const std::string & p,
                         STG::Tariffs & t,
                         STG::Users & u)
            : TableSensor(p),
              tariffs(t),
              users(u)
        {}

        void UpdateSensors(Sensors & sensors) const override;

    private:
        STG::Tariffs & tariffs;
        STG::Users & users;
};

typedef std::map<std::string, TableSensor *> Tables;
}
