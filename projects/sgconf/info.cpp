#include "info.h"

#include "api_action.h"
#include "options.h"
#include "makeproto.h"
#include "config.h"

#include "stg/servconf.h"

#include <iostream>
#include <string>
#include <map>

#include <expat.h>

namespace
{

void PrintInfo(const STG::ServerInfo::Info& info)
{
    std::cout << "Server version: '" << info.version << "'\n"
              << "Number of tariffs: " << info.tariffNum << "\n"
              << "Tariff subsystem version: " << info.tariffType << "\n"
              << "Number of users: " << info.usersNum << "\n"
              << "UName: '" << info.uname << "\n"
              << "Number of directions: " << info.dirNum << "\n"
              << "Dirs:\n";
    for (size_t i = 0; i < info.dirName.size(); ++i)
        std::cout << "\t - '" << info.dirName[i] << "'\n";
}

void InfoCallback(bool result, const std::string & reason, const STG::ServerInfo::Info & info, void * /*data*/)
{
if (!result)
    {
    std::cerr << "Failed to get server info. Reason: '" << reason << "'." << std::endl;
    return;
    }
PrintInfo(info);
}

bool InfoFunction(const SGCONF::CONFIG & config,
                  const std::string& /*arg*/,
                  const std::map<std::string, std::string> & /*options*/)
{
return makeProto(config).ServerInfo(InfoCallback, NULL) == STG::st_ok;
}

}

void SGCONF::AppendServerInfoBlock(COMMANDS & commands, OPTION_BLOCKS & blocks)
{
blocks.Add("Server info")
      .Add("server-info", SGCONF::MakeAPIAction(commands, InfoFunction), "\tget server info");
}
