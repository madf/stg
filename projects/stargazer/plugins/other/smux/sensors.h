#ifndef __SENSORS_H__
#define __SENSORS_H__

#include <map>

#include "stg/users.h"
#include "stg/tariffs.h"
#include "stg/user_property.h"

#include "stg/ObjectSyntax.h"

#include "value2os.h"
#include "types.h"

class Sensor {
    public:
        virtual bool GetValue(ObjectSyntax_t * objectSyntax) const = 0;
};

typedef std::map<OID, Sensor *> Sensors;

class TableSensor {
    public:
        virtual bool appendTable(Sensors & sensors);
};

class TotalUsersSensor : public Sensor {
    public:
        TotalUsersSensor(const USERS & u) : users(u) {}
        virtual ~TotalUsersSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const
        {
        ValueToOS(users.GetUserNum(), objectSyntax);
        return true;
        }

    private:
        const USERS & users;
};

class UsersSensor : public Sensor {
    public:
        UsersSensor(USERS & u) : users(u) {}
        virtual ~UsersSensor() {};

        bool GetValue(ObjectSyntax_t * objectSyntax) const;

    private:
        USERS & users;

        virtual bool UserPredicate(USER_PTR userPtr) const = 0;
};

class ConnectedUsersSensor : public UsersSensor {
    public:
        ConnectedUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~ConnectedUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetConnected(); }
};

class AuthorizedUsersSensor : public UsersSensor {
    public:
        AuthorizedUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~AuthorizedUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetAuthorized(); }
};

class AlwaysOnlineUsersSensor : public UsersSensor {
    public:
        AlwaysOnlineUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~AlwaysOnlineUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetProperty().alwaysOnline; }
};

class NoCashUsersSensor : public UsersSensor {
    public:
        NoCashUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~NoCashUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetProperty().cash < 0; }
};

class DisabledDetailStatsUsersSensor : public UsersSensor {
    public:
        DisabledDetailStatsUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~DisabledDetailStatsUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetProperty().disabledDetailStat; }
};

class DisabledUsersSensor : public UsersSensor {
    public:
        DisabledUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~DisabledUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetProperty().disabled; }
};

class PassiveUsersSensor : public UsersSensor {
    public:
        PassiveUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~PassiveUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetProperty().passive; }
};

class CreditUsersSensor : public UsersSensor {
    public:
        CreditUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~CreditUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetProperty().credit > 0; }
};

class FreeMbUsersSensor : public UsersSensor {
    public:
        FreeMbUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~FreeMbUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetProperty().freeMb > 0; }
};

class TariffChangeUsersSensor : public UsersSensor {
    public:
        TariffChangeUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~TariffChangeUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetProperty().nextTariff.ConstData().empty(); }
};

class TotalTariffsSensor : public Sensor {
    public:
        TotalTariffsSensor(const TARIFFS & t) : tariffs(t) {}
        virtual ~TotalTariffsSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const
        {
        ValueToOS(tariffs.GetTariffsNum(), objectSyntax);
        return true;
        }

    private:
        const TARIFFS & tariffs;
};

template <typename T>
class ConstSensor : public Sensor {
    public:
        ConstSensor(const T & v) : value(v) {}
        virtual ~ConstSensor() {}

        bool GetValue(ObjectSyntax * objectSyntax) const
        { return ValueToOS(value, objectSyntax); }

    private:
        T value;
};

#endif
