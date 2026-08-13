#pragma once

#include "radproto/dictionaries.h"
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

            struct ASectionData
            {
                std::string attrName;
                uint32_t attrCode;
                std::string attrType;
                AttrValue value;
            };

            struct ASection
            {
                using ASectionDataVect = std::vector<ASectionData>;
                ASectionDataVect match;
                ASectionDataVect send;
            };

            const std::string& GetStrError() const { return m_errorStr; }
            int ParseSettings(const ModuleSettings& s);

            uint16_t GetPort() const { return m_port; }
            const std::string& GetDictionaries() const { return m_dictionariesPath; }
            const std::string& GetSecret() const { return m_secret; }
            const ASection& getAuth() const { return m_auth; }
            const ASection& getAutz() const { return m_autz; }

        private:
            std::vector<ASectionData> ParseRules(const std::string& value, const std::string& paramName);
            ASection parseASection(const std::vector<ParamValue>& conf);

            std::string m_errorStr;
            uint16_t m_port;
            std::string m_dictionariesPath;
            std::string m_secret;
            RadProto::Dictionaries m_dictionaries;

            ASection m_auth;
            ASection m_autz;

            PluginLogger m_logger;
    };
}
