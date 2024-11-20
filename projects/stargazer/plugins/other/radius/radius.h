#pragma once

#include "stg/auth.h"
#include <string>

namespace STG
{
    class RADIUS : public Auth
    {
        public:
            RADIUS();

            int Start() override { return 0; }
            int Stop() override { return 0; }
            int Reload(const ModuleSettings & /*ms*/) override { return 0; }
            bool IsRunning() override { return isRunning; }
            int ParseSettings() override { return 0; }
            const std::string & GetStrError() const override { return errorStr; }
            std::string GetVersion() const override;
            uint16_t GetStartPosition() const override { return 0; }
            uint16_t GetStopPosition() const override { return 0; }

            int SendMessage(const Message & msg, uint32_t ip) const override { return 0; }

        private:
            mutable std::string errorStr;
            bool isRunning;

    };
}
