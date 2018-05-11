#pragma once

#include "cqhttp/logging/handler.h"

#include <spdlog/spdlog.h>

namespace cqhttp::logging {
    struct SpdlogHandler : Handler {
        void log(cq::logging::Level level, const std::string &tag, const std::string &msg) const override;

    protected:
        std::shared_ptr<spdlog::logger> logger_ = nullptr;
    };
} // namespace cqhttp::logging
