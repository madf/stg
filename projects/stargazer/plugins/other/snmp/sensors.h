#ifndef __SENSORS_H__
#define __SENSORS_H__

#include <string>
#include <map>

#include "stg/users.h"
#include "stg/tariffs.h"

#include "asn1/ObjectSyntax.h"

class Sensor {
    public:
        virtual bool GetValue(ObjectSyntax_t * objectSyntax) = 0;
};

typedef std::map<std::string, Sensor *> Sensors;

void Int2OS(ObjectSyntax_t * dst, long src);

class TotalUsersSensor : public Sensor {
    public:
        TotalUsersSensor(const USERS & u)
            : users(u)
        {}
        virtual ~TotalUsersSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax)
        {
        Int2OS(objectSyntax, users.GetUserNum());
        return true;
        }

    private:
        const USERS & users;
};

class ConnectedUsersSensor : public Sensor {
    public:
        ConnectedUsersSensor(USERS & u)
            : users(u)
        {}
        virtual ~ConnectedUsersSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax);

    private:
        USERS & users;
};

class AuthorizedUsersSensor : public Sensor {
    public:
        AuthorizedUsersSensor(USERS & u)
            : users(u)
        {}
        virtual ~AuthorizedUsersSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax);

    private:
        USERS & users;
};

class AlwaysOnlineUsersSensor : public Sensor {
    public:
        AlwaysOnlineUsersSensor(USERS & u)
            : users(u)
        {}
        virtual ~AlwaysOnlineUsersSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax);

    private:
        USERS & users;
};

class NoCashUsersSensor : public Sensor {
    public:
        NoCashUsersSensor(USERS & u)
            : users(u)
        {}
        virtual ~NoCashUsersSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax);

    private:
        USERS & users;
};

class DisabledDetailStatsUsersSensor : public Sensor {
    public:
        DisabledDetailStatsUsersSensor(USERS & u)
            : users(u)
        {}
        virtual ~DisabledDetailStatsUsersSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax);

    private:
        USERS & users;
};

class DisabledUsersSensor : public Sensor {
    public:
        DisabledUsersSensor(USERS & u)
            : users(u)
        {}
        virtual ~DisabledUsersSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax);

    private:
        USERS & users;
};

class PassiveUsersSensor : public Sensor {
    public:
        PassiveUsersSensor(USERS & u)
            : users(u)
        {}
        virtual ~PassiveUsersSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax);

    private:
        USERS & users;
};

class CreditUsersSensor : public Sensor {
    public:
        CreditUsersSensor(USERS & u)
            : users(u)
        {}
        virtual ~CreditUsersSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax);

    private:
        USERS & users;
};

class FreeMbUsersSensor : public Sensor {
    public:
        FreeMbUsersSensor(USERS & u)
            : users(u)
        {}
        virtual ~FreeMbUsersSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax);

    private:
        USERS & users;
};

class TotalTariffsSensor : public Sensor {
    public:
        TotalTariffsSensor(const TARIFFS & t)
            : tariffs(t)
        {}
        virtual ~TotalTariffsSensor() {}

        bool GetValue(ObjectSyntax_t * objectSyntax)
        {
        Int2OS(objectSyntax, tariffs.GetTariffsNum());
        return true;
        }

    private:
        const TARIFFS & tariffs;
};

#endif
