#include "corps.h"

#include "api_action.h"
#include "options.h"
#include "makeproto.h"
#include "config.h"
#include "utils.h"

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

void PrintCorp(const STG::GetCorp::Info & info, size_t level = 0)
{
std::cout << Indent(level, true) << "name: " << info.name << "\n"
          << Indent(level)       << "cash: " << info.cash << "\n";
}

std::vector<SGCONF::API_ACTION::PARAM> GetCorpParams()
{
std::vector<SGCONF::API_ACTION::PARAM> params;
params.push_back(SGCONF::API_ACTION::PARAM("cash", "<cash>", "\tcorporation's cash"));
return params;
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
                      const std::vector<STG::GetCorp::Info> & info,
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
                     const STG::GetCorp::Info & info,
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
return makeProto(config).GetCorporations(GetCorpsCallback, NULL) == STG::st_ok;
}

bool GetCorpFunction(const SGCONF::CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & /*options*/)
{
return makeProto(config).GetCorp(arg, GetCorpCallback, NULL) == STG::st_ok;
}

bool DelCorpFunction(const SGCONF::CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & /*options*/)
{
return makeProto(config).DelCorp(arg, SimpleCallback, NULL) == STG::st_ok;
}

bool AddCorpFunction(const SGCONF::CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & options)
{
STG::CorpConfOpt conf;
conf.name = arg;
SGCONF::MaybeSet(options, "cash", conf.cash);
return makeProto(config).AddCorp(arg, conf, SimpleCallback, NULL) == STG::st_ok;
}

bool ChgCorpFunction(const SGCONF::CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & options)
{
STG::CorpConfOpt conf;
conf.name = arg;
SGCONF::MaybeSet(options, "cash", conf.cash);
return makeProto(config).ChgCorp(conf, SimpleCallback, NULL) == STG::st_ok;
}

} // namespace anonymous

void SGCONF::AppendCorpsOptionBlock(COMMANDS & commands, OPTION_BLOCKS & blocks)
{
std::vector<API_ACTION::PARAM> params(GetCorpParams());
blocks.Add("Corporation management options")
      .Add("get-corps", SGCONF::MakeAPIAction(commands, GetCorpsFunction), "\tget corporation list")
      .Add("get-corp", SGCONF::MakeAPIAction(commands, "<name>", GetCorpFunction), "get corporation")
      .Add("add-corp", SGCONF::MakeAPIAction(commands, "<name>", params, AddCorpFunction), "add corporation")
      .Add("del-corp", SGCONF::MakeAPIAction(commands, "<name>", DelCorpFunction), "delete corporation")
      .Add("chg-corp", SGCONF::MakeAPIAction(commands, "<name>", params, ChgCorpFunction), "change corporation");
}
