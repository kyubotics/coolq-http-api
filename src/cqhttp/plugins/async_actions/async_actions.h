#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct AsyncActions : Plugin {
        std::string name() const override { return "async_actions"; }
        void hook_missed_action(ActionContext &ctx) override;
    };

    static std::shared_ptr<AsyncActions> async_actions = std::make_shared<AsyncActions>();
} // namespace cqhttp::plugins
