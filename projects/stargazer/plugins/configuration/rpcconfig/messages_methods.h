#pragma once

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>

namespace STG
{

class Users;

}

class RPC_CONFIG;

class METHOD_MESSAGE_SEND : public xmlrpc_c::method {
public:
    METHOD_MESSAGE_SEND(RPC_CONFIG * c,
                     STG::Users * u)
        : config(c),
          users(u)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value * const retvalPtr);

private:
    METHOD_MESSAGE_SEND(const METHOD_MESSAGE_SEND & rvalue);
    METHOD_MESSAGE_SEND & operator=(const METHOD_MESSAGE_SEND & rvalue);

    RPC_CONFIG * config;
    STG::Users * users;
};
