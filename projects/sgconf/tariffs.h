#ifndef __STG_SGCONF_TARIFFS_H__
#define __STG_SGCONF_TARIFFS_H__

#include <string>
#include <map>

namespace SGCONF
{

struct CONFIG;

bool GetTariffsFunction(const CONFIG & config,
                        const std::string & /*arg*/,
                        const std::map<std::string, std::string> & /*options*/);

bool GetTariffFunction(const CONFIG & config,
                       const std::string & arg,
                       const std::map<std::string, std::string> & /*options*/);

bool DelTariffFunction(const CONFIG & config,
                       const std::string & arg,
                       const std::map<std::string, std::string> & /*options*/);

bool AddTariffFunction(const CONFIG & config,
                       const std::string & arg,
                       const std::map<std::string, std::string> & options);

bool ChgTariffFunction(const CONFIG & config,
                       const std::string & arg,
                       const std::map<std::string, std::string> & options);

} // namespace SGCONF

#endif
