#pragma once

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>

#include <string>
#include <vector>

namespace STG
{

struct Settings;
class Users;
class Tariffs;

}

// Forward declaration
class RPC_CONFIG;

class METHOD_INFO : public xmlrpc_c::method
{
public:
    METHOD_INFO(STG::Tariffs * t,
                STG::Users * u,
                size_t df,
                const std::vector<std::string> & dn)
        : tariffs(t),
          users(u),
          dayFee(df),
          dirNames(dn)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value *   const   retvalP);

private:
    METHOD_INFO(const METHOD_INFO & rvalue);
    METHOD_INFO & operator=(const METHOD_INFO & rvalue);

    STG::Tariffs * tariffs;
    STG::Users * users;
    size_t dayFee;
    const std::vector<std::string> & dirNames;
};

class METHOD_LOGIN : public xmlrpc_c::method
{
public:
    explicit METHOD_LOGIN(RPC_CONFIG * c)
        : config(c)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value *   const   retvalP);

private:
    METHOD_LOGIN(const METHOD_LOGIN & rvalue);
    METHOD_LOGIN & operator=(const METHOD_LOGIN & rvalue);

    RPC_CONFIG * config;
};

class METHOD_LOGOUT : public xmlrpc_c::method
{
public:
    explicit METHOD_LOGOUT(RPC_CONFIG * c)
        : config(c)
    {
    }

    void execute(xmlrpc_c::paramList const & paramList,
                 xmlrpc_c::value *   const   retvalP);

private:
    METHOD_LOGOUT(const METHOD_LOGOUT & rvalue);
    METHOD_LOGOUT & operator=(const METHOD_LOGOUT & rvalue);

    RPC_CONFIG * config;
};
