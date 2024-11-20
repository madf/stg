#include "radius.h"

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

