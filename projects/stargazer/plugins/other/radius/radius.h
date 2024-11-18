#pragma once

#include "stg/auth.h"
#include "stg/user.h"
#include "stg/logger.h"

namespace STG
{
class Users;

using UserPtr = User*;
using ConstUserPtr = const User*;

class RADIUS : public Auth
{
    public:
        RADIUS();

        std::string GetVersion() const override;
        int SendMessage(const Message & msg, uint32_t ip) const override;

        void SetUsers(Users * u) override { users = u; }

        int Start() override;


};
