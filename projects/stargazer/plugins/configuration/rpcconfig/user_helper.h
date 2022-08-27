#pragma once

#include <string>

#include <xmlrpc-c/base.hpp>

namespace STG
{

class Admin;
struct Store;
class Tariffs;
class User;
class Users;

}

class USER_HELPER
{
public:
    using UserPtr = STG::User*;
    USER_HELPER(UserPtr & p, STG::Users & us)
        : ptr(p),
          users(us)
    {
    }

    void GetUserInfo(xmlrpc_c::value * info,
                     bool hidePassword = false);
    bool SetUserInfo(const xmlrpc_c::value & info,
                     const STG::Admin& admin,
                     const std::string & login,
                     const STG::Store & store,
                     STG::Tariffs * tariffs);
private:
    UserPtr & ptr;
    STG::Users & users;
};
