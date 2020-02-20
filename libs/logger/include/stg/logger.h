#pragma once

#include <string>
#include <mutex>

namespace STG
{

class Logger
{
    public:
        void setFileName(const std::string& fn);
        void operator()(const char * fmt, ...) const;
        void operator()(const std::string & line) const { logString(line.c_str()); }

        static Logger& get();

    private:
        const char* logDate(time_t t) const;
        void logString(const char* str) const;

        mutable std::mutex mutex;
        std::string fileName;
};
//-----------------------------------------------------------------------------
class PluginLogger
{
    public:
        static PluginLogger get(std::string pluginName)
        {
            return PluginLogger(std::move(pluginName));
        }

        PluginLogger(PluginLogger&& rhs)
            : m_parent(Logger::get()),
              m_pluginName(std::move(rhs.m_pluginName))
        {}
        PluginLogger& operator=(PluginLogger&& rhs)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_pluginName = std::move(rhs.m_pluginName);
            return *this;
        }

        void operator()(const char* fmt, ...) const;
        void operator()(const std::string& line) const;

    private:
        explicit PluginLogger(std::string pn)
            : m_parent(Logger::get()),
              m_pluginName(std::move(pn))
        {}

        mutable std::mutex m_mutex;
        Logger& m_parent;
        std::string m_pluginName;
};

}
