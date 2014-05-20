#ifndef __STG_SGCONF_ADMINS_H__
#define __STG_SGCONF_ADMINS_H__

#include <string>
#include <map>

namespace SGCONF
{

class CONFIG;

bool GetAdminsFunction(const CONFIG & config,
                       const std::string & /*arg*/,
                       const std::map<std::string, std::string> & /*options*/);

bool GetAdminFunction(const CONFIG & config,
                      const std::string & arg,
                      const std::map<std::string, std::string> & /*options*/);

bool DelAdminFunction(const CONFIG & config,
                      const std::string & arg,
                      const std::map<std::string, std::string> & /*options*/);

bool AddAdminFunction(const CONFIG & config,
                      const std::string & arg,
                      const std::map<std::string, std::string> & options);

bool ChgAdminFunction(const CONFIG & config,
                      const std::string & arg,
                      const std::map<std::string, std::string> & options);

} // namespace SGCONF

#endif
