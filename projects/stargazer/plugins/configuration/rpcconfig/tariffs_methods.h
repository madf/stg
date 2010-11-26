#ifndef __TARIFFS_METHODS_H__
#define __TARIFFS_METHODS_H__

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>

class RPC_CONFIG;
class TARIFFS;
class USERS;
class ADMINS;

class METHOD_TARIFF_GET : public xmlrpc_c::method {
public:
    METHOD_TARIFF_GET(RPC_CONFIG * c,
                      TARIFFS * t)
        : config(c),
          tariffs(t)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value *   const   retvalPtr);
private:
    RPC_CONFIG * config;
    TARIFFS * tariffs;
};

class METHOD_TARIFF_CHG : public xmlrpc_c::method {
public:
    METHOD_TARIFF_CHG(RPC_CONFIG * c,
                      ADMINS * a,
                      TARIFFS * t)
        : config(c),
          admins(a),
          tariffs(t)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value *   const   retvalPtr);
private:
    RPC_CONFIG * config;
    ADMINS * admins;
    TARIFFS * tariffs;
};

class METHOD_TARIFFS_GET : public xmlrpc_c::method {
public:
    METHOD_TARIFFS_GET(RPC_CONFIG * c,
                      TARIFFS * t)
        : config(c),
          tariffs(t)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value *   const   retvalPtr);
private:
    RPC_CONFIG * config;
    TARIFFS * tariffs;
};

class METHOD_TARIFF_ADD : public xmlrpc_c::method {
public:
    METHOD_TARIFF_ADD(RPC_CONFIG * c,
                      ADMINS * a,
                      TARIFFS * t)
        : config(c),
          admins(a),
          tariffs(t)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value *   const   retvalP);
private:
    RPC_CONFIG * config;
    ADMINS * admins;
    TARIFFS * tariffs;
};

class METHOD_TARIFF_DEL : public xmlrpc_c::method {
public:
    METHOD_TARIFF_DEL(RPC_CONFIG * c,
                      ADMINS * a,
                      TARIFFS * t,
                      USERS * u)
        : config(c),
          admins(a),
          tariffs(t),
          users(u)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value *   const   retvalP);
private:
    RPC_CONFIG * config;
    ADMINS * admins;
    TARIFFS * tariffs;
    USERS * users;
};

#endif
