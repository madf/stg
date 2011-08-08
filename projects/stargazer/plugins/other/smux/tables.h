#ifndef __TABLES_H__
#define __TABLES_H__

#include <string>
#include <map>

#include "sensors.h"

class TableSensor {
    public:
        TableSensor(const std::string & p) : prefix(p) {}
        virtual ~TableSensor() {}

        const std::string & GetPrefix() const { return prefix; }
        virtual void UpdateSensors(Sensors & sensors) const = 0;

    protected:
        std::string prefix;
};

class TariffUsersTable : public TableSensor {
    public:
        TariffUsersTable(const std::string & p,
                         USERS & u)
            : TableSensor(p),
              users(u)
        {}
        virtual ~TariffUsersTable() {}

        void UpdateSensors(Sensors & sensors) const;

    private:
        USERS & users;
};

typedef std::map<std::string, TableSensor *> Tables;

#endif
