#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>
#include <locale>

namespace STG
{

class IsNotSpace : public std::unary_function<bool, char> {
public:
    IsNotSpace(const std::locale & l) : loc(l) {};
    bool operator() (char c)
        {
        return !std::use_facet<casefacet>(loc).is(std::ctype_base::space, c);
        };
private:
    const std::locale & loc;

    typedef std::ctype<char> casefacet;
};

class ToLowerHelper : public std::unary_function<char, char> {
public:
    ToLowerHelper(const std::locale & l) : loc(l) {};
    char operator() (char c)
        {
        return std::tolower(c, loc);
        };
private:
    const std::locale & loc;
};

class ToUpperHelper : public std::unary_function<char, char> {
public:
    ToUpperHelper(const std::locale & l) : loc(l) {};
    char operator() (char c)
        {
        return std::toupper(c, loc);
        };
private:
    const std::locale & loc;
};

std::string Trim(const std::string & val, const std::locale & loc);
std::string ToLower(const std::string & val, const std::locale & loc);
std::string ToUpper(const std::string & val, const std::locale & loc);

inline std::string Trim(const std::string & val)
    {
    return Trim(val, std::locale(""));
    }

inline std::string ToLower(const std::string & val)
    {
    return ToLower(val, std::locale(""));
    }

inline std::string ToUpper(const std::string & val)
    {
    return ToUpper(val, std::locale(""));
    }

}

std::string inet_ntostring(uint32_t ip);

#endif
