#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <iostream>
#include <string>

#define LOG_IT (log << __FILE__ << ":" << __LINE__ << " ")

class STGLogger {
public:
    STGLogger() : out(std::cout) {};
    STGLogger(std::ostream & stream) : out(stream) {};
    ~STGLogger();

    std::ostream &operator <<(const std::string & val);
private:
    void LogDate();
    std::ostream & out;
};

extern STGLogger log;

#endif
