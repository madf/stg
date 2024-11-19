#include "radius.h"
#include "stg/user.h"
#include "stg/users.h"

#include <functional> // mem_fun_ref
#include <cassert>


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

void RADIUS::GetUsers()
{
    UserPtr u;
    int h = users->OpenSearch();
    assert(h && "USERS::OpenSearch is always correct");

    while (!users->SearchNext(h, &u))
        {
            userList.push_back(u);
            SetUserNotifiers(u);
        }

    users->CloseSearch(h);
}

void RADIUS::UpdateUserAuthorization(ConstUserPtr u) const
{
    if (u->GetProperties().alwaysOnline)
    {
        auto ips = u->GetProperties().ips.get();
        if (ips.onlyOneIP())
        {
            users->Authorize(u->GetLogin(), ips[0].ip, 0xFFffFFff, this);
        }
    }
}

void RADIUS::AddUser(UserPtr u)
{
    SetUserNotifiers(u);
    userList.push_back(u);
    UpdateUserAuthorization(u);
}

void RADIUS::DelUser(UserPtr u)
{
    if (u->IsAuthorizedBy(this))
        users->Unauthorize(u->GetLogin(), this);
    UnSetUserNotifiers(u);
    userList.erase(std::remove(userList.begin(), userList.end(), u), userList.end());
}

int RADIUS::SendMessage(const Message &, uint32_t) const
{
    errorStr = "Authorization modele \'Radius\' does not support sending messages";
    return -1;
}

