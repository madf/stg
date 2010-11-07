#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>

std::string IconvString(const std::string & src,
                        const std::string & from = "UTF-8",
                        const std::string & to = "KOI8-R");

#endif
