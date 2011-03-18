#ifndef __INFO_METHODS_H__
#define __INFO_METHODS_H__

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>

#include "users.h"
#include "tariffs.h"

// Forward declaration
class RPC_CONFIG;
class SETTINGS;

class METHOD_INFO : public xmlrpc_c::method
{
public:
    METHOD_INFO(TARIFFS * t,
                USERS * u,
                const SETTINGS * s)
        : tariffs(t),
          users(u),
          settings(s)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value *   const   retvalP);
private:
    TARIFFS * tariffs;
    USERS * users;
    const SETTINGS * settings;
};

class METHOD_LOGIN : public xmlrpc_c::method
{
public:
    METHOD_LOGIN(RPC_CONFIG * c)
        : config(c)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value *   const   retvalP);
private:
    RPC_CONFIG * config;
};

class METHOD_LOGOUT : public xmlrpc_c::method
{
public:
    METHOD_LOGOUT(RPC_CONFIG * c)
        : config(c)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value *   const   retvalP);
private:
    RPC_CONFIG * config;
};

#endif
