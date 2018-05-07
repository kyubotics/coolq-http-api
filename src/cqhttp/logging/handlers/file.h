#pragma once

#include "cqhttp/logging/handler.h"

#include <spdlog/logger.h>

namespace cqhttp::logging {
    struct FileHandler : IHandler {
        FileHandler();
        void log(cq::logging::Level level, const std::string &tag, const std::string &msg) const override;

    private:
        std::shared_ptr<spdlog::logger> logger_;
    };
} // namespace cqhttp::logging
