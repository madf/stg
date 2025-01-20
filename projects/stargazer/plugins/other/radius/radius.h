#pragma once

#include "stg/auth.h"
#include "stg/logger.h"

#include <string>
#include <mutex>
#include <jthread.hpp>

namespace STG
{
    class RADIUS : public Auth
    {
        public:
            RADIUS();

            int Start() override;
            int Stop() override;
            int Reload(const ModuleSettings & /*ms*/) override { return 0; }
            bool IsRunning() override { return m_running; }
            void SetRunning(bool val);
            int ParseSettings() override { return 0; }
            const std::string & GetStrError() const override { return m_errorStr; }
            std::string GetVersion() const override;
            uint16_t GetStartPosition() const override { return 0; }
            uint16_t GetStopPosition() const override { return 0; }

            int SendMessage(const Message & msg, uint32_t ip) const override { return 0; }

        private:
            std::mutex m_mutex;
            mutable std::string m_errorStr;
            std::jthread m_thread;
            bool m_running;

            int Run(std::stop_token token);
            PluginLogger m_logger;
    };
}
