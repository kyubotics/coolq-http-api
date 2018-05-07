#pragma once

#include "cqhttp/core/plugin.h"

#include "cqhttp/plugins/async_actions/vendor/ctpl/ctpl_stl.h"

namespace cqhttp::plugins {
    struct AsyncActions : Plugin {
        void hook_enable(Context &ctx) override;
        void hook_disable(Context &ctx) override;
        void hook_missed_action(ActionContext &ctx) override;

    private:
        std::shared_ptr<ctpl::thread_pool> pool_;
    };

    static std::shared_ptr<AsyncActions> async_actions = std::make_shared<AsyncActions>();
} // namespace cqhttp::plugins
