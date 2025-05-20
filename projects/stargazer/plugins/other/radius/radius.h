#pragma once

#include "stg/auth.h"
#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/subscriptions.h"
#include "stg/logger.h"
#include "server.h"

#include <boost/asio.hpp>
#include <string>
#include <memory>
#include <mutex>
#include <jthread.hpp>
#include <cstdint> //uint8_t, uint32_t

namespace STG
{
    struct Settings;

    class Users;

    class RAD_SETTINGS
    {
        public:
            RAD_SETTINGS();
            virtual ~RAD_SETTINGS() {}

            struct AttrValue
            {
                enum class Type
                {
                    PARAM_NAME,
                    VALUE
                };
                std::string value;
                Type type;
            };

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
            RADIUS(const RADIUS&) = delete;
            RADIUS& operator=(const RADIUS&) = delete;

            void SetUsers(Users* u) { m_users = u; }
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
            std::mutex m_mutex;

            boost::asio::io_service m_ioService;
            int Run(std::stop_token token);

            mutable std::string m_errorStr;
            RAD_SETTINGS m_radSettings;
            ModuleSettings m_settings;

            bool m_running;

            std::jthread m_thread;
            Users* m_users;
            PluginLogger m_logger;

            std::unique_ptr<Server> m_server;
    };
}
