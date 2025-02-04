#pragma once

#include "stg/auth.h"
#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/subscriptions.h"
#include "stg/logger.h"

#include <string>
#include <mutex>
#include <jthread.hpp>
#include <cstdint> //uint8_t, uint32_t

namespace STG
{
    struct Settings;

    class RAD_SETTINGS
    {
        public:
            RAD_SETTINGS();
            virtual ~RAD_SETTINGS() {}
            const std::string & GetStrError() const { return m_errorStr; }
            int ParseSettings(const ModuleSettings & s);

            uint16_t GetPort() const { return m_port; }
            const std::string & GetDictionaries() const { return m_dictionaries; }
            const std::string & GetSecret() const { return m_secret; }

        private:
            std::string m_errorStr;
            uint16_t m_port;
            std::string m_dictionaries;
            std::string m_secret;
    };

    class RADIUS : public Auth
    {
        public:
            RADIUS();

            void SetSettings(const ModuleSettings & s) override { m_settings = s; }
            int ParseSettings() override;

            int Start() override;
            int Stop() override;
            int Reload(const ModuleSettings & /*ms*/) override { return 0; }
            bool IsRunning() override;
            void SetRunning(bool val);

            const std::string & GetStrError() const override { return m_errorStr; }
            std::string GetVersion() const override;

            uint16_t GetStartPosition() const override { return 0; }
            uint16_t GetStopPosition() const override { return 0; }

            int SendMessage(const Message & msg, uint32_t ip) const override { return 0; }

        private:
            RADIUS(const RADIUS & rhs);
            RADIUS & operator=(const RADIUS & rhs);

            int Run(std::stop_token token);

            mutable std::string m_errorStr;
            RAD_SETTINGS m_radSettings;
            ModuleSettings m_settings;

            bool m_running;

            std::jthread m_thread;
            std::mutex m_mutex;

            PluginLogger m_logger;
    };
}
