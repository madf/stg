#include <algorithm>
#include <functional>

#include <iostream>
#include <cerrno>
#include <cstring>

#include <arpa/inet.h>

#include "utils.h"

using namespace std;

string STG::ToLower(const string & val, const locale & loc)
{
    std::string res;
    transform(val.begin(),
	      val.end(),
	      back_inserter(res),
	      STG::ToLowerHelper(loc));
    return res;
}

string STG::ToUpper(const string & val, const locale & loc)
{
    std::string res;
    transform(val.begin(),
	      val.end(),
	      back_inserter(res),
	      STG::ToUpperHelper(loc));
    return res;
}

string STG::Trim(const string & val, const locale & loc)
{
    if (val.empty())
        return std::string();
    string::const_iterator first(find_if(
		val.begin(),
		val.end(),
		STG::IsNotSpace(loc)));
    string::const_reverse_iterator last(find_if(
		val.rbegin(),
		val.rend(),
		STG::IsNotSpace(loc)));
    if (first == val.end())
        return std::string();
    return std::string(first, last.base());
}
std::string inet_ntostring(uint32_t ip)
{
    char buf[INET_ADDRSTRLEN + 1];

    return inet_ntop(AF_INET, &ip, buf, INET_ADDRSTRLEN);
}
