#ifndef CORP_CONF_H
#define CORP_CONF_H

#include <string>

struct CORP_CONF
{
CORP_CONF() : name(), cash(0) {}
CORP_CONF(const std::string & n) : name(n), cash(0) {}
CORP_CONF(const std::string & n, double c) : name(n), cash(c) {}

std::string name;
double      cash;
};

inline
bool operator==(const CORP_CONF & a, const CORP_CONF & b)
{
return a.name == b.name;
}

#endif //CORP_CONF_H
