#ifndef __MESSAGES_METHODS_H__
#define __MESSAGES_METHODS_H__

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>

#include "stg/users.h"

class RPC_CONFIG;

class METHOD_MESSAGE_SEND : public xmlrpc_c::method {
public:
    METHOD_MESSAGE_SEND(RPC_CONFIG * c,
                     USERS * u)
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
    USERS * users;
};

#endif
