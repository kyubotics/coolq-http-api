#pragma once

#include "cqhttp/logging/handler.h"

#include <spdlog/spdlog.h>

namespace cqhttp::logging {
    struct SpdlogHandler : Handler {
    protected:
        std::shared_ptr<spdlog::logger> logger_ = nullptr;
        static spdlog::level::level_enum convert_level(cq::logging::Level level);
    };
} // namespace cqhttp::logging
