#pragma once

#include "cqhttp/logging/handlers/_spdlog.h"

namespace cqhttp::logging {
    struct ConsoleHandler : SpdlogHandler {
        void init() override;
        void destroy() override;
    };
} // namespace cqhttp::logging
