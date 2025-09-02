#pragma once

#include "stg/auth.h"
#include "stg/plugin.h"
#include "config.h"
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
    class Users;

    class RADIUS : public Auth
    {
        public:
            RADIUS();
            RADIUS(const RADIUS&) = delete;
            RADIUS& operator=(const RADIUS&) = delete;

            void SetUsers(Users* u) override { m_users = u; }
            void SetSettings(const ModuleSettings& s) override { m_settings = s; }
            int ParseSettings() override;

            int Start() override;
            int Stop() override;
            int Reload(const ModuleSettings& /*ms*/) override { return 0; }
            bool IsRunning() override;
            void SetRunning(bool val);

            const std::string& GetStrError() const override { return m_errorStr; }
            std::string GetVersion() const override;

            uint16_t GetStartPosition() const override { return 0; }
            uint16_t GetStopPosition() const override { return 0; }

            int SendMessage(const Message& /*msg*/, uint32_t /*ip*/) const override { return 0; }

        private:
            std::mutex m_mutex;

            boost::asio::io_context m_ioContext;
            int Run(std::stop_token token);

            mutable std::string m_errorStr;
            Config m_config;
            ModuleSettings m_settings;

            bool m_running;

            std::jthread m_thread;
            Users* m_users;
            PluginLogger m_logger;

            std::unique_ptr<Server> m_server;
    };
}
