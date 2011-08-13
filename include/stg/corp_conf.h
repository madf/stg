#ifndef CORP_CONF_H
#define CORP_CONF_H

#include <string>

struct CORP_CONF
{
CORP_CONF(const std::string & n) : name(n), cash(0) {}
CORP_CONF(const std::string & n, double c) : name(n), cash(c) {}

std::string name;
double      cash;
};

#endif //CORP_CONF_H
