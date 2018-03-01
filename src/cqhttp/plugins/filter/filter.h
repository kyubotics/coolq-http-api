#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct Filter : Plugin {};

    static std::shared_ptr<Filter> filter = std::make_shared<Filter>();
}  // namespace cqhttp::plugins
