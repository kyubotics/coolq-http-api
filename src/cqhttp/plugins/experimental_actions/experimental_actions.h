#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct ExperimentalActions : Plugin {
        void hook_missed_action(ActionContext &ctx) override;
    };

    static std::shared_ptr<ExperimentalActions> experimental_actions = std::make_shared<ExperimentalActions>();
} // namespace cqhttp::plugins
