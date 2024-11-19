#pragma once

#include "stg/auth.h"
#include "stg/user.h"
#include "stg/logger.h"

#include <string>
#include <vector>
#include <list>

namespace STG
{
    class Users;

    using UserPtr = User*;
    using ConstUserPtr = const User*;

    class RADIUS : public Auth
    {
        public:
            RADIUS();

            void SetUsers(Users * u) override { users = u; }

            int Start() override;
            int Stop() override;
            std::string GetVersion() const override;

            int SendMessage(const Message & msg, uint32_t ip) const override;
        private:
            void AddUser(UserPtr u);
            void DelUser(UserPtr u);

            void GetUsers();
            void SetUserNotifiers(UserPtr u);
            void UnSetUserNotifiers(UserPtr u);
            void UpdateUserAuthorization(ConstUserPtr u) const;

            mutable std::string errorStr;
            Users* users;
            std::vector<UserPtr> userList;
            bool isRunning;

            using ConnHolder = std::tuple<int, ScopedConnection, ScopedConnection, ScopedConnection, ScopedConnection>;
            std::vector<ConnHolder> m_conns;

            ScopedConnection m_onAddUserConn;
            ScopedConnection m_onDelUserConn;



    };

}
