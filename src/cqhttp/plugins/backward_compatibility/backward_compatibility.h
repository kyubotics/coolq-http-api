#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct BackwardCompatibility : Plugin {
        void hook_event(EventContext<cq::Event> &ctx) override;
    };

    static std::shared_ptr<BackwardCompatibility> backward_compatibility = std::make_shared<BackwardCompatibility>();
} // namespace cqhttp::plugins
