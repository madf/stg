#pragma once

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>

namespace STG
{

struct Admins;

}

class RPC_CONFIG;

class METHOD_ADMIN_GET : public xmlrpc_c::method {
public:
    METHOD_ADMIN_GET(RPC_CONFIG * c,
                     STG::Admins * a)
        : config(c),
          admins(a)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value * const retvalPtr);

private:
    METHOD_ADMIN_GET(const METHOD_ADMIN_GET & rvalue);
    METHOD_ADMIN_GET & operator=(const METHOD_ADMIN_GET & rvalue);

    RPC_CONFIG * config;
    STG::Admins * admins;
};

class METHOD_ADMIN_ADD : public xmlrpc_c::method {
public:
    METHOD_ADMIN_ADD(RPC_CONFIG * c,
                     STG::Admins * a)
        : config(c),
          admins(a)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value * const retvalPtr);

private:
    METHOD_ADMIN_ADD(const METHOD_ADMIN_ADD & rvalue);
    METHOD_ADMIN_ADD & operator=(const METHOD_ADMIN_ADD & rvalue);

    RPC_CONFIG * config;
    STG::Admins * admins;
};

class METHOD_ADMIN_DEL : public xmlrpc_c::method {
public:
    METHOD_ADMIN_DEL(RPC_CONFIG * c,
                     STG::Admins * a)
        : config(c),
          admins(a)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value * const retvalPtr);

private:
    METHOD_ADMIN_DEL(const METHOD_ADMIN_DEL & rvalue);
    METHOD_ADMIN_DEL & operator=(const METHOD_ADMIN_DEL & rvalue);

    RPC_CONFIG * config;
    STG::Admins * admins;
};

class METHOD_ADMIN_CHG : public xmlrpc_c::method {
public:
    METHOD_ADMIN_CHG(RPC_CONFIG * c,
                     STG::Admins * a)
        : config(c),
          admins(a)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value * const retvalPtr);

private:
    METHOD_ADMIN_CHG(const METHOD_ADMIN_CHG & rvalue);
    METHOD_ADMIN_CHG & operator=(const METHOD_ADMIN_CHG & rvalue);

    RPC_CONFIG * config;
    STG::Admins * admins;
};

class METHOD_ADMINS_GET : public xmlrpc_c::method {
public:
    METHOD_ADMINS_GET(RPC_CONFIG * c,
                      STG::Admins * a)
        : config(c),
          admins(a)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value * const retvalPtr);

private:
    METHOD_ADMINS_GET(const METHOD_ADMINS_GET & rvalue);
    METHOD_ADMINS_GET & operator=(const METHOD_ADMINS_GET & rvalue);

    RPC_CONFIG * config;
    STG::Admins * admins;
};
