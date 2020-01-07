#ifndef __STG_DUMP_HELPERS_H__
#define __STG_DUMP_HELPERS_H__

#include "stg/common.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <cstddef>
#include <ctime>

namespace STG
{

class Dumper
{
    public:
        explicit Dumper(const std::string& tag)
            : m_stream(getName(tag).c_str())
        {
        }
        ~Dumper() {}

        void write(const void* data, size_t size)
        {
            writePrefix();
            m_stream << " ";
            writeHEX(data, size);
        }

    private:
        std::ofstream m_stream;

        tm getTime() const
        {
            time_t now = time(NULL);
            tm localTime;
            localtime_r(&now, &localTime);
            return localTime;
        }

        std::string getName(const std::string& tag) const
        {
            tm localTime = getTime();

            std::ostringstream res;
            res << tag
                << "-" << (localTime.tm_year + 1900) << twoDigit(localTime.tm_mon + 1) << twoDigit(localTime.tm_mday)
                << "-" << twoDigit(localTime.tm_hour) << twoDigit(localTime.tm_min) << twoDigit(localTime.tm_sec)
                << ".data";

            return res.str();
        }

        void writePrefix()
        {
            tm localTime = getTime();
            m_stream << "[" << (localTime.tm_year + 1900) << "-" << twoDigit(localTime.tm_mon + 1) << "-" << twoDigit(localTime.tm_mday)
                     << " " << twoDigit(localTime.tm_hour) << ":" << twoDigit(localTime.tm_min) << ":" << twoDigit(localTime.tm_sec)
                     << "]";
        }

        void writeHEX(const void* data, size_t size)
        {
            m_stream << "(" << std::setw(4) << std::setfill(' ') << size << ") ";
            const unsigned char* pos = static_cast<const unsigned char*>(data);
            for (size_t i = 0; i < size; ++i)
                m_stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(*pos++);
            m_stream << std::dec << "\n";
        }

        std::string twoDigit(int value) const
        {
            std::string res = x2str(value);
            if (res.length() < 2)
                res = "0" + res;
            return res;
        }
};

} // namespace Caster

#endif
