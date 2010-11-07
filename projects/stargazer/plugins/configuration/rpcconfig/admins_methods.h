#ifndef __ADMINS_METHODS_H__
#define __ADMINS_METHODS_H__

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>

#include "../../../admins.h"
#include "../../../admin.h"

class RPC_CONFIG;

class METHOD_ADMIN_GET : public xmlrpc_c::method {
public:
    METHOD_ADMIN_GET(RPC_CONFIG * c,
                     ADMINS * a)
        : config(c),
          admins(a)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value * const retvalPtr);
private:
    RPC_CONFIG * config;
    ADMINS * admins;
};

class METHOD_ADMIN_ADD : public xmlrpc_c::method {
public:
    METHOD_ADMIN_ADD(RPC_CONFIG * c,
                     ADMINS * a)
        : config(c),
          admins(a)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value * const retvalPtr);
private:
    RPC_CONFIG * config;
    ADMINS * admins;
};

class METHOD_ADMIN_DEL : public xmlrpc_c::method {
public:
    METHOD_ADMIN_DEL(RPC_CONFIG * c,
                     ADMINS * a)
        : config(c),
          admins(a)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value * const retvalPtr);
private:
    RPC_CONFIG * config;
    ADMINS * admins;
};

class METHOD_ADMIN_CHG : public xmlrpc_c::method {
public:
    METHOD_ADMIN_CHG(RPC_CONFIG * c,
                     ADMINS * a)
        : config(c),
          admins(a)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value * const retvalPtr);
private:
    RPC_CONFIG * config;
    ADMINS * admins;
};

class METHOD_ADMINS_GET : public xmlrpc_c::method {
public:
    METHOD_ADMINS_GET(RPC_CONFIG * c,
                      ADMINS * a)
        : config(c),
          admins(a)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value * const retvalPtr);
private:
    RPC_CONFIG * config;
    ADMINS * admins;
};

#endif
