#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct WorkerPoolResizer : Plugin {
        std::string name() const override { return "worker_pool_resizer"; }
        void hook_enable(Context &ctx) override;
    };

    static std::shared_ptr<WorkerPoolResizer> worker_pool_resizer = std::make_shared<WorkerPoolResizer>();
} // namespace cqhttp::plugins
