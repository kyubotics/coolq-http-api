#pragma once

#include "cqhttp/logging/handlers/_spdlog.h"

namespace cqhttp::logging {
    struct FileHandler : SpdlogHandler {
        void init() override;
        void destroy() override;
    };
} // namespace cqhttp::logging
