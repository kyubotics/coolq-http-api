#pragma once

#include "cqsdk/logging.h"

namespace cqhttp::logging {
    struct Handler {
        virtual ~Handler() = default;
        virtual void log(cq::logging::Level level, const std::string &tag, const std::string &msg) = 0;
    };
}
