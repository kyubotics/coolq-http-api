#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct HeartbeatGenerator : Plugin {
        void hook_enable(Context &ctx) override;
    };

    static std::shared_ptr<HeartbeatGenerator> heartbeat_generator = std::make_shared<HeartbeatGenerator>();
} // namespace cqhttp::plugins
