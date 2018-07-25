#pragma once

#include "cqhttp/logging/handlers/_spdlog.h"

namespace cqhttp::logging {
    struct ConsoleHandler : SpdlogHandler {
        void init() override;
        void destroy() override;
        void log(cq::logging::Level level, const std::string &tag, const std::string &msg) const override;
    };
} // namespace cqhttp::logging
