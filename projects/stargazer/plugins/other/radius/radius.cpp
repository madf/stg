#include "radius.h"
#include "stg/user.h"
#include "stg/users.h"

#include <functional> // mem_fun_ref


using STG::RADIUS;

extern "C" STG::Plugin* GetPlugin()
{
    static RADIUS plugin;
    return &plugin;
}

std::string RADIUS::GetVersion() const
{
    return "Radius authorizator v.1.0";
}

RADIUS::RADIUS()
{
}

int RADIUS::Start()
{
    printfd(__FILE__, "RADIUS::Start()\n");
    GetUsers();

    m_onAddUserConn = users->onAdd([this](auto user){ AddUser(user); });
    m_onDelUserConn = users->onDel([this](auto user){ DelUser(user); });

    std::for_each(userList.begin(), userList.end(), [this](auto user){ UpdateUserAuthorization(user); });

    isRunning = true;

    return 0;
}

int RADIUS::Stop()
{
    printfd(__FILE__, "RADIUS::Stop()\n");
    if (!isRunning)
        return 0;

    m_onAddUserConn.disconnect();
    m_onDelUserConn.disconnect();

    m_conns.clear();

    isRunning = false;
    return 0;
}




int RADIUS::SendMessage(const Message &, uint32_t) const
{
    errorStr = "Authorization modele \'Radius\' does not support sending messages";
    return -1;
}

