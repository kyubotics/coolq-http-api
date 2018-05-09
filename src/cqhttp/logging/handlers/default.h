#pragma once

#include "cqhttp/logging/handler.h"

namespace cqhttp::logging {
    struct DefaultHandler : Handler {
        void log(const cq::logging::Level level, const std::string &tag, const std::string &msg) const override {
            cq::logging::log(level, tag, msg);
        }
    };
} // namespace cqhttp::logging
