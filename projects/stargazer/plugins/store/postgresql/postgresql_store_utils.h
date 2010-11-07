#ifndef POSTGRESQL_UTILS_STORE_H
#define POSTGRESQL_UTILS_STORE_H

#include <functional>

struct ToLower : public std::unary_function<char, char>
{
char operator() (char c) const  { return std::tolower(c); }
};

#endif
