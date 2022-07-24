#pragma once

#include "config.h"

#include "stg/servconf.h"

namespace SGCONF
{

inline
STG::ServConf makeProto(const CONFIG& config)
{
    return STG::ServConf(config.server.value(),
                         config.port.value(),
                         config.localAddress.value(),
                         config.localPort.value(),
                         config.userName.value(),
                         config.userPass.value());
}

}
