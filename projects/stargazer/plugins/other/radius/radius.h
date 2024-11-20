#pragma once

#include "stg/auth.h"

#include <string>

namespace STG
{

    class RADIUS : public Auth
    {
        public:
            RADIUS();

            int Start() override;
            int Stop() override;
            std::string GetVersion() const override;

            int SendMessage(const Message & msg, uint32_t ip) const override;

    };

}
