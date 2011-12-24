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
    METHOD_TARIFF_GET(const METHOD_TARIFF_GET & rvalue);
    METHOD_TARIFF_GET & operator=(const METHOD_TARIFF_GET & rvalue);

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
    METHOD_TARIFF_CHG(const METHOD_TARIFF_CHG & rvalue);
    METHOD_TARIFF_CHG & operator=(const METHOD_TARIFF_CHG & rvalue);

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
    METHOD_TARIFFS_GET(const METHOD_TARIFFS_GET & rvalue);
    METHOD_TARIFFS_GET & operator=(const METHOD_TARIFFS_GET & rvalue);

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
    METHOD_TARIFF_ADD(const METHOD_TARIFF_ADD & rvalue);
    METHOD_TARIFF_ADD & operator=(const METHOD_TARIFF_ADD & rvalue);

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
    METHOD_TARIFF_DEL(const METHOD_TARIFF_DEL & rvalue);
    METHOD_TARIFF_DEL & operator=(const METHOD_TARIFF_DEL & rvalue);

    RPC_CONFIG * config;
    ADMINS * admins;
    TARIFFS * tariffs;
    USERS * users;
};

#endif
