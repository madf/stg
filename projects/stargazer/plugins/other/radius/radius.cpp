#include "radius.h"
#include "stg/users.h"


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

int RADIUS::SendMessage(const Message &, uint32_t) const
{
errorStr = "Authorization modele \'Radius\' does not support sending messages";
return -1;
}

