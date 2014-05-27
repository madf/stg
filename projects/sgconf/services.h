#ifndef __STG_SGCONF_SERVICES_H__
#define __STG_SGCONF_SERVICES_H__

#include <string>
#include <map>

namespace SGCONF
{

struct CONFIG;

bool GetServicesFunction(const CONFIG & config,
                         const std::string & /*arg*/,
                         const std::map<std::string, std::string> & /*options*/);

bool GetServiceFunction(const CONFIG & config,
                        const std::string & arg,
                        const std::map<std::string, std::string> & /*options*/);

bool DelServiceFunction(const CONFIG & config,
                        const std::string & arg,
                        const std::map<std::string, std::string> & /*options*/);

bool AddServiceFunction(const CONFIG & config,
                        const std::string & arg,
                        const std::map<std::string, std::string> & options);

bool ChgServiceFunction(const CONFIG & config,
                        const std::string & arg,
                        const std::map<std::string, std::string> & options);

} // namespace SGCONF

#endif
