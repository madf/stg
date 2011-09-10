#ifndef SERVICE_CONF_H
#define SERVICE_CONF_H

#include <string>

#include "os_int.h"

struct SERVICE_CONF
{
SERVICE_CONF()
    : name(), comment(), cost(0), payDay(0)
{}
SERVICE_CONF(const std::string & n)
    : name(n), comment(), cost(0), payDay(0)
{}
SERVICE_CONF(const std::string & n, double c)
    : name(n), comment(), cost(c), payDay(0)
{}
SERVICE_CONF(const std::string & n, double c, unsigned p)
    : name(n), comment(), cost(c), payDay(p)
{}
SERVICE_CONF(const std::string & n, double c,
             unsigned p, const std::string & com)
    : name(n), comment(com), cost(c), payDay(p)
{}

std::string name;
std::string comment;
double      cost;
uint8_t     payDay;
};

inline
bool operator==(const SERVICE_CONF & a, const SERVICE_CONF & b)
{
return a.name == b.name;
}

#endif //SERVICE_CONF_H

