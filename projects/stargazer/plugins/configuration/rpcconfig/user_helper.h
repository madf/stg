#ifndef __USER_HELPER_H__
#define __USER_HELPER_H__

#include <string>

#include <xmlrpc-c/base.hpp>

#include "users.h"

class ADMIN;
class STORE;
class TARIFFS;

class USER_HELPER
{
public:
    USER_HELPER(user_iter & it)
        : iter(it)
    {
    }

    void GetUserInfo(xmlrpc_c::value * info,
                     bool hidePassword = false);
    bool SetUserInfo(const xmlrpc_c::value & info,
                     const ADMIN & admin,
                     const std::string & login,
                     const STORE & store,
                     TARIFFS * tariffs);
private:
    user_iter & iter;
};

#endif
