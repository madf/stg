#include "services.h"

#include "api_action.h"
#include "options.h"
#include "config.h"

#include "stg/servconf.h"
#include "stg/servconf_types.h"
#include "stg/service_conf.h"
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

void PrintService(const STG::GET_SERVICE::INFO & info, size_t level = 0)
{
std::cout << Indent(level, true) << "name: " << info.name << "\n"
          << Indent(level)       << "cost: " << info.cost << "\n"
          << Indent(level)       << "payment day: " << info.payDay << "\n"
          << Indent(level)       << "comment: " << info.comment << "\n";
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

void GetServicesCallback(bool result,
                         const std::string & reason,
                         const std::vector<STG::GET_SERVICE::INFO> & info,
                         void * /*data*/)
{
if (!result)
    {
    std::cerr << "Failed to get service list. Reason: '" << reason << "'." << std::endl;
    return;
    }
std::cout << "Services:\n";
for (size_t i = 0; i < info.size(); ++i)
    PrintService(info[i], 1);
}

void GetServiceCallback(bool result,
                        const std::string & reason,
                        const STG::GET_SERVICE::INFO & info,
                        void * /*data*/)
{
if (!result)
    {
    std::cerr << "Failed to get service. Reason: '" << reason << "'." << std::endl;
    return;
    }
PrintService(info);
}

bool GetServicesFunction(const SGCONF::CONFIG & config,
                         const std::string & /*arg*/,
                         const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.GetServices(GetServicesCallback, NULL) == STG::st_ok;
}

bool GetServiceFunction(const SGCONF::CONFIG & config,
                        const std::string & arg,
                        const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.GetService(arg, GetServiceCallback, NULL) == STG::st_ok;
}

bool DelServiceFunction(const SGCONF::CONFIG & config,
                        const std::string & arg,
                        const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.DelService(arg, SimpleCallback, NULL) == STG::st_ok;
}

bool AddServiceFunction(const SGCONF::CONFIG & config,
                        const std::string & arg,
                        const std::map<std::string, std::string> & /*options*/)
{
// TODO
std::cerr << "Unimplemented.\n";
return false;
}

bool ChgServiceFunction(const SGCONF::CONFIG & config,
                        const std::string & arg,
                        const std::map<std::string, std::string> & options)
{
// TODO
std::cerr << "Unimplemented.\n";
return false;
}

} // namespace anonymous

void SGCONF::AppendServicesOptionBlock(COMMANDS & commands, OPTION_BLOCKS & blocks)
{
blocks.Add("Service management options")
      .Add("get-services", SGCONF::MakeAPIAction(commands, GetServicesFunction), "\tget service list")
      .Add("get-service", SGCONF::MakeAPIAction(commands, "<name>", true, GetServiceFunction), "get service")
      .Add("add-service", SGCONF::MakeAPIAction(commands, "<name>", true, AddServiceFunction), "add service")
      .Add("del-service", SGCONF::MakeAPIAction(commands, "<name>", true, DelServiceFunction), "del service")
      .Add("chg-service", SGCONF::MakeAPIAction(commands, "<name>", true, ChgServiceFunction), "change service");
}
