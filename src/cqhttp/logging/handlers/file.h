#pragma once

#include "cqhttp/logging/handler.h"

#include <spdlog/logger.h>

namespace cqhttp::logging {
    struct FileHandler : Handler {
        void init() override;
        void destroy() override;
        void log(cq::logging::Level level, const std::string &tag, const std::string &msg) const override;

    private:
        std::shared_ptr<spdlog::logger> logger_ = nullptr;
    };
} // namespace cqhttp::logging
