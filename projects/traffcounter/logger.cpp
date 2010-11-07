#include <string>
#include <sstream>

#include <ctime>

#include "logger.h"

using namespace std;

STGLogger::~STGLogger()
{
}

ostream & STGLogger::operator <<(const string & val)
{
    LogDate();
    out << " " << val;
    return out;
}

void STGLogger::LogDate()
{
    time_t t(time(NULL));
    struct tm * tt = localtime(&t);
    out << "[" << tt->tm_year + 1900 << "-";
    out << (tt->tm_mon + 1 < 10 ? "0" : "") << tt->tm_mon + 1 << "-";
    out << (tt->tm_mday < 10 ? "0" : "") << tt->tm_mday << " ";
    out << (tt->tm_hour < 10 ? "0" : "") << tt->tm_hour << ":";
    out << (tt->tm_min < 10 ? "0" : "") << tt->tm_min << ":";
    out << (tt->tm_sec < 10 ? "0" : "") << tt->tm_sec << "]";
}
