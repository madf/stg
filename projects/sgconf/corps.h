#ifndef __STG_SGCONF_CORPS_H__
#define __STG_SGCONF_CORPS_H__

#include <string>
#include <map>

namespace SGCONF
{

struct CONFIG;

bool GetCorpsFunction(const CONFIG & config,
                      const std::string & /*arg*/,
                      const std::map<std::string, std::string> & /*options*/);

bool GetCorpFunction(const CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & /*options*/);

bool DelCorpFunction(const CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & /*options*/);

bool AddCorpFunction(const CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & options);

bool ChgCorpFunction(const CONFIG & config,
                     const std::string & arg,
                     const std::map<std::string, std::string> & options);

} // namespace SGCONF

#endif
