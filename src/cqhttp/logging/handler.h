#pragma once

#include "cqsdk/logging.h"

namespace cqhttp::logging {
    struct IHandler {
        virtual ~IHandler() = default;
        virtual void log(cq::logging::Level level, const std::string &tag, const std::string &msg) const = 0;
    };
}
