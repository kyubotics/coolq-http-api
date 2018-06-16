#pragma once

#include "cqhttp/logging/handlers/_spdlog.h"

namespace cqhttp::logging {
    struct FileHandler : SpdlogHandler {
        FileHandler(const std::string &log_filename) : log_filename_(log_filename) {}
        void init() override;
        void destroy() override;

    private:
        std::string log_filename_;
    };
} // namespace cqhttp::logging
