#ifndef __SENSORS_H__
#define __SENSORS_H__

#include <map>

#include "stg/users.h"
#include "stg/tariffs.h"
#include "stg/admins.h"
#include "stg/services.h"
#include "stg/corporations.h"
#include "stg/traffcounter.h"
#include "stg/user_property.h"

#include "stg/ObjectSyntax.h"

#include "value2os.h"
#include "types.h"

class Sensor {
    public:
        virtual ~Sensor() {}
        virtual bool GetValue(ObjectSyntax_t * objectSyntax) const = 0;
#ifdef DEBUG
        virtual std::string ToString() const = 0;
#endif
};

typedef std::map<OID, Sensor *> Sensors;

class TotalUsersSensor : public Sensor {
    public:
        explicit TotalUsersSensor(const USERS & u) : users(u) {}
        virtual ~TotalUsersSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const
        {
        ValueToOS(users.Count(), objectSyntax);
        return true;
        }

#ifdef DEBUG
        std::string ToString() const
        { std::string res; std::to_string(users.Count(), res); return res; }
#endif

    private:
        const USERS & users;
};

class UsersSensor : public Sensor {
    public:
        explicit UsersSensor(USERS & u) : users(u) {}
        virtual ~UsersSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const;
#ifdef DEBUG
        std::string ToString() const;
#endif

    private:
        USERS & users;

        virtual bool UserPredicate(USER_PTR userPtr) const = 0;
};

class ConnectedUsersSensor : public UsersSensor {
    public:
        explicit ConnectedUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~ConnectedUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetConnected(); }
};

class AuthorizedUsersSensor : public UsersSensor {
    public:
        explicit AuthorizedUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~AuthorizedUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetAuthorized(); }
};

class AlwaysOnlineUsersSensor : public UsersSensor {
    public:
        explicit AlwaysOnlineUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~AlwaysOnlineUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetProperty().alwaysOnline; }
};

class NoCashUsersSensor : public UsersSensor {
    public:
        explicit NoCashUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~NoCashUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetProperty().cash < 0; }
};

class DisabledDetailStatsUsersSensor : public UsersSensor {
    public:
        explicit DisabledDetailStatsUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~DisabledDetailStatsUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetProperty().disabledDetailStat; }
};

class DisabledUsersSensor : public UsersSensor {
    public:
        explicit DisabledUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~DisabledUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetProperty().disabled; }
};

class PassiveUsersSensor : public UsersSensor {
    public:
        explicit PassiveUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~PassiveUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetProperty().passive; }
};

class CreditUsersSensor : public UsersSensor {
    public:
        explicit CreditUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~CreditUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetProperty().credit > 0; }
};

class FreeMbUsersSensor : public UsersSensor {
    public:
        explicit FreeMbUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~FreeMbUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return userPtr->GetProperty().freeMb > 0; }
};

class TariffChangeUsersSensor : public UsersSensor {
    public:
        explicit TariffChangeUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~TariffChangeUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const
        { return !userPtr->GetProperty().nextTariff.ConstData().empty(); }
};

class ActiveUsersSensor : public UsersSensor {
    public:
        explicit ActiveUsersSensor(USERS & u) : UsersSensor(u) {}
        virtual ~ActiveUsersSensor() {}

    private:
        bool UserPredicate(USER_PTR userPtr) const;
};

class TotalTariffsSensor : public Sensor {
    public:
        explicit TotalTariffsSensor(const TARIFFS & t) : tariffs(t) {}
        virtual ~TotalTariffsSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const
        {
        ValueToOS(tariffs.Count(), objectSyntax);
        return true;
        }

#ifdef DEBUG
        std::string ToString() const
        { std::string res; std::to_string(tariffs.Count(), res); return res; }
#endif

    private:
        const TARIFFS & tariffs;
};

class TotalAdminsSensor : public Sensor {
    public:
        explicit TotalAdminsSensor(const ADMINS & a) : admins(a) {}
        virtual ~TotalAdminsSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const
        {
        ValueToOS(admins.Count(), objectSyntax);
        return true;
        }

#ifdef DEBUG
        std::string ToString() const
        { std::string res; std::to_string(admins.Count(), res); return res; }
#endif

    private:
        const ADMINS & admins;
};

class TotalServicesSensor : public Sensor {
    public:
        explicit TotalServicesSensor(const SERVICES & s) : services(s) {}
        virtual ~TotalServicesSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const
        {
        ValueToOS(services.Count(), objectSyntax);
        return true;
        }

#ifdef DEBUG
        std::string ToString() const
        { std::string res; std::to_string(services.Count(), res); return res; }
#endif

    private:
        const SERVICES & services;
};

class TotalCorporationsSensor : public Sensor {
    public:
        explicit TotalCorporationsSensor(const CORPORATIONS & c) : corporations(c) {}
        virtual ~TotalCorporationsSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const
        {
        ValueToOS(corporations.Count(), objectSyntax);
        return true;
        }

#ifdef DEBUG
        std::string ToString() const
        { std::string res; std::to_string(corporations.Count(), res); return res; }
#endif

    private:
        const CORPORATIONS & corporations;
};

class TotalRulesSensor : public Sensor {
    public:
        explicit TotalRulesSensor(const TRAFFCOUNTER & t) : traffcounter(t) {}
        virtual ~TotalRulesSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const
        {
        ValueToOS(traffcounter.RulesCount(), objectSyntax);
        return true;
        }

#ifdef DEBUG
        std::string ToString() const
        { std::string res; std::to_string(traffcounter.RulesCount(), res); return res; }
#endif

    private:
        const TRAFFCOUNTER & traffcounter;
};

template <typename T>
class ConstSensor : public Sensor {
    public:
        explicit ConstSensor(const T & v) : value(v) {}
        virtual ~ConstSensor() {}

        bool GetValue(ObjectSyntax * objectSyntax) const
        { return ValueToOS(value, objectSyntax); }

#ifdef DEBUG
        std::string ToString() const
        { std::string res; std::to_string(value, res); return res; }
#endif

    private:
        T value;
};

#ifdef DEBUG
template <>
inline
std::string ConstSensor<std::string>::ToString() const
{
return value;
}
#endif

#endif
