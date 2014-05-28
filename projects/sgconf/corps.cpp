#include "corps.h"

#include "api_action.h"
#include "options.h"
#include "config.h"

#include "stg/servconf.h"
#include "stg/servconf_types.h"
#include "stg/corp_conf.h"
#include "stg/common.h"

#include <iostream>
#include <string>
#include <map>

namespace
{

std::string Indent(size_t level, bool dash = false)
{
if (level == 0)
    return "";
return dash ? std::string(level * 4 - 2, ' ') + "- " : std::string(level * 4, ' ');
}

void PrintCorp(const STG::GET_CORP::INFO & info, size_t level = 0)
{
std::cout << Indent(level, true) << "name: " << info.name << "\n"
          << Indent(level)       << "cash: " << info.cash << "\n";
}

void SimpleCallback(bool result,
                    const std::string & reason,
                    void * /*data*/)
{
if (!result)
    {
    std::cerr << "Operation failed. Reason: '" << reason << "'." << std::endl;
    return;
    }
std::cout << "Success.\n";
}

void GetCorpsCallback(bool result,
                      const std::string & reason,
                      const std::vector<STG::GET_CORP::INFO> & info,
                      void * /*data*/)
{
if (!result)
    {
    std::cerr << "Failed to get corp list. Reason: '" << reason << "'." << std::endl;
    return;
    }
std::cout << "Corps:\n";
for (size_t i = 0; i < info.size(); ++i)
    PrintCorp(info[i], 1);
}

void GetCorpCallback(bool result,
                     const std::string & reason,
                     const STG::GET_CORP::INFO & info,
                     void * /*data*/)
{
if (!result)
    {
    std::cerr << "Failed to get corp. Reason: '" << reason << "'." << std::endl;
    return;
    }
PrintCorp(info);
}

bool GetCorpsFunction(const SGCONF::CONFIG & config,
                      const std::string & /*arg*/,
                      const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.GetCorporations(GetCorpsCallback, NULL) == STG::st_ok;
}

bool GetCorpFunction(const SGCONF::CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.GetCorp(arg, GetCorpCallback, NULL) == STG::st_ok;
}

bool DelCorpFunction(const SGCONF::CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.DelCorp(arg, SimpleCallback, NULL) == STG::st_ok;
}

bool AddCorpFunction(const SGCONF::CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & /*options*/)
{
// TODO
std::cerr << "Unimplemented.\n";
return false;
}

bool ChgCorpFunction(const SGCONF::CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & options)
{
// TODO
std::cerr << "Unimplemented.\n";
return false;
}

} // namespace anonymous

void SGCONF::AppendCorpsOptionBlock(COMMANDS & commands, OPTION_BLOCKS & blocks)
{
blocks.Add("Corporation management options")
      .Add("get-corps", SGCONF::MakeAPIAction(commands, GetCorpsFunction), "\tget corporation list")
      .Add("get-corp", SGCONF::MakeAPIAction(commands, "<name>", GetCorpFunction), "get corporation")
      .Add("add-corp", SGCONF::MakeAPIAction(commands, "<name>", AddCorpFunction), "add corporation")
      .Add("del-corp", SGCONF::MakeAPIAction(commands, "<name>", DelCorpFunction), "del corporation")
      .Add("chg-corp", SGCONF::MakeAPIAction(commands, "<name>", ChgCorpFunction), "change corporation");
}
