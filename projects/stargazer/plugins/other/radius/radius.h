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
};
