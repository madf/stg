#ifndef __STG_SGCONF_USERS_H__
#define __STG_SGCONF_USERS_H__

#include <string>
#include <map>

namespace SGCONF
{

struct CONFIG;

bool GetUsersFunction(const CONFIG & config,
                      const std::string & /*arg*/,
                      const std::map<std::string, std::string> & /*options*/);

bool GetUserFunction(const CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & /*options*/);

bool DelUserFunction(const CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & /*options*/);

bool AddUserFunction(const CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & options);

bool ChgUserFunction(const CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & options);

}

#endif
