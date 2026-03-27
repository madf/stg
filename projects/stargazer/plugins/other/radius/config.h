#pragma once

#include "stg/module_settings.h"
#include "stg/subscriptions.h"
#include "stg/logger.h"

#include <string>
#include <cstdint> //uint8_t, uint32_t

namespace STG
{
    struct Settings;

    class Config
    {
        public:
            Config();

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

            struct ASection
            {
                using Pairs = std::vector<std::pair<std::string, AttrValue>>;
                Pairs match;
                Pairs send;
            };

            const std::string& GetStrError() const { return m_errorStr; }
            int ParseSettings(const ModuleSettings& s);

            uint16_t GetPort() const { return m_port; }
            const std::string& GetDictionaries() const { return m_dictionaries; }
            const std::string& GetSecret() const { return m_secret; }
            const ASection& getAuth() const { return m_auth; }
            const ASection& getAutz() const { return m_autz; }

        private:
            std::vector<std::pair<std::string, AttrValue>> ParseRules(const std::string& value, const std::string& paramName);
            ASection parseASection(const std::vector<ParamValue>& conf);

            std::string m_errorStr;
            uint16_t m_port;
            std::string m_dictionaries;
            std::string m_secret;

            ASection m_auth;
            ASection m_autz;

            PluginLogger m_logger;
    };
}
