#ifndef __SENSORS_H__
#define __SENSORS_H__

#include <map>

#include "stg/users.h"
#include "stg/user.h"
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
        virtual ~Sensor() = default;
        virtual bool GetValue(ObjectSyntax_t * objectSyntax) const = 0;
#ifdef DEBUG
        virtual std::string ToString() const = 0;
#endif
};

typedef std::map<OID, Sensor *> Sensors;

class TotalUsersSensor : public Sensor {
    public:
        explicit TotalUsersSensor(const STG::Users & u) : users(u) {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const override
        {
        ValueToOS(users.Count(), objectSyntax);
        return true;
        }

#ifdef DEBUG
        std::string ToString() const override
        { std::string res; std::to_string(users.Count(), res); return res; }
#endif

    private:
        const STG::Users & users;
};

class UsersSensor : public Sensor {
    public:
        explicit UsersSensor(STG::Users & u) : users(u) {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const override;
#ifdef DEBUG
        std::string ToString() const override;
#endif

    private:
        STG::Users & users;

        virtual bool UserPredicate(STG::User* userPtr) const = 0;
};

class ConnectedUsersSensor : public UsersSensor {
    public:
        explicit ConnectedUsersSensor(STG::Users & u) : UsersSensor(u) {}

    private:
        bool UserPredicate(STG::User* userPtr) const override
        { return userPtr->GetConnected(); }
};

class AuthorizedUsersSensor : public UsersSensor {
    public:
        explicit AuthorizedUsersSensor(STG::Users & u) : UsersSensor(u) {}

    private:
        bool UserPredicate(STG::User* userPtr) const override
        { return userPtr->GetAuthorized(); }
};

class AlwaysOnlineUsersSensor : public UsersSensor {
    public:
        explicit AlwaysOnlineUsersSensor(STG::Users & u) : UsersSensor(u) {}

    private:
        bool UserPredicate(STG::User* userPtr) const override
        { return userPtr->GetProperties().alwaysOnline; }
};

class NoCashUsersSensor : public UsersSensor {
    public:
        explicit NoCashUsersSensor(STG::Users & u) : UsersSensor(u) {}

    private:
        bool UserPredicate(STG::User* userPtr) const override
        { return userPtr->GetProperties().cash < 0; }
};

class DisabledDetailStatsUsersSensor : public UsersSensor {
    public:
        explicit DisabledDetailStatsUsersSensor(STG::Users & u) : UsersSensor(u) {}

    private:
        bool UserPredicate(STG::User* userPtr) const override
        { return userPtr->GetProperties().disabledDetailStat; }
};

class DisabledUsersSensor : public UsersSensor {
    public:
        explicit DisabledUsersSensor(STG::Users & u) : UsersSensor(u) {}

    private:
        bool UserPredicate(STG::User* userPtr) const override
        { return userPtr->GetProperties().disabled; }
};

class PassiveUsersSensor : public UsersSensor {
    public:
        explicit PassiveUsersSensor(STG::Users & u) : UsersSensor(u) {}

    private:
        bool UserPredicate(STG::User* userPtr) const override
        { return userPtr->GetProperties().passive; }
};

class CreditUsersSensor : public UsersSensor {
    public:
        explicit CreditUsersSensor(STG::Users & u) : UsersSensor(u) {}

    private:
        bool UserPredicate(STG::User* userPtr) const override
        { return userPtr->GetProperties().credit > 0; }
};

class FreeMbUsersSensor : public UsersSensor {
    public:
        explicit FreeMbUsersSensor(STG::Users & u) : UsersSensor(u) {}

    private:
        bool UserPredicate(STG::User* userPtr) const override
        { return userPtr->GetProperties().freeMb > 0; }
};

class TariffChangeUsersSensor : public UsersSensor {
    public:
        explicit TariffChangeUsersSensor(STG::Users & u) : UsersSensor(u) {}

    private:
        bool UserPredicate(STG::User* userPtr) const override
        { return !userPtr->GetProperties().nextTariff.ConstData().empty(); }
};

class ActiveUsersSensor : public UsersSensor {
    public:
        explicit ActiveUsersSensor(STG::Users & u) : UsersSensor(u) {}

    private:
        bool UserPredicate(STG::User* userPtr) const override;
};

class TotalTariffsSensor : public Sensor {
    public:
        explicit TotalTariffsSensor(const STG::Tariffs & t) : tariffs(t) {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const override
        {
        ValueToOS(tariffs.Count(), objectSyntax);
        return true;
        }

#ifdef DEBUG
        std::string ToString() const override
        { std::string res; std::to_string(tariffs.Count(), res); return res; }
#endif

    private:
        const STG::Tariffs & tariffs;
};

class TotalAdminsSensor : public Sensor {
    public:
        explicit TotalAdminsSensor(const STG::Admins & a) : admins(a) {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const override
        {
        ValueToOS(admins.count(), objectSyntax);
        return true;
        }

#ifdef DEBUG
        std::string ToString() const override
        { std::string res; std::to_string(admins.Count(), res); return res; }
#endif

    private:
        const STG::Admins & admins;
};

class TotalServicesSensor : public Sensor {
    public:
        explicit TotalServicesSensor(const STG::Services & s) : services(s) {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const override
        {
        ValueToOS(services.Count(), objectSyntax);
        return true;
        }

#ifdef DEBUG
        std::string ToString() const override
        { std::string res; std::to_string(services.Count(), res); return res; }
#endif

    private:
        const STG::Services & services;
};

class TotalCorporationsSensor : public Sensor {
    public:
        explicit TotalCorporationsSensor(const STG::Corporations & c) : corporations(c) {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const override
        {
        ValueToOS(corporations.Count(), objectSyntax);
        return true;
        }

#ifdef DEBUG
        std::string ToString() const override
        { std::string res; std::to_string(corporations.Count(), res); return res; }
#endif

    private:
        const STG::Corporations & corporations;
};

class TotalRulesSensor : public Sensor {
    public:
        explicit TotalRulesSensor(const STG::TraffCounter & t) : traffcounter(t) {}

        bool GetValue(ObjectSyntax_t * objectSyntax) const override
        {
        ValueToOS(traffcounter.rulesCount(), objectSyntax);
        return true;
        }

#ifdef DEBUG
        std::string ToString() const override
        { std::string res; std::to_string(traffcounter.rulesCount(), res); return res; }
#endif

    private:
        const STG::TraffCounter & traffcounter;
};

template <typename T>
class ConstSensor : public Sensor {
    public:
        explicit ConstSensor(const T & v) : value(v) {}

        bool GetValue(ObjectSyntax * objectSyntax) const override
        { return ValueToOS(value, objectSyntax); }

#ifdef DEBUG
        std::string ToString() const override
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
