#pragma once

#include "cqhttp/core/plugin.h"

#include <atomic>
#include <thread>

#include "cqhttp/utils/channel.h"

namespace cqhttp::plugins {
    struct RateLimitedActions : Plugin {
        std::string name() const override { return "rate_limited_actions"; }
        void hook_enable(Context &ctx) override;
        void hook_disable(Context &ctx) override;
        void hook_missed_action(ActionContext &ctx) override;
        bool good() const override { return !enabled_ || worker_running_; }

    private:
        struct QueuedContext {
            std::string action;
            json params;
        };

        std::shared_ptr<Channel<QueuedContext>> chan_;
        std::thread worker_thread_;
        std::atomic_bool worker_running_ = false;
        bool enabled_ = false;
        std::chrono::milliseconds interval_;
    };

    static std::shared_ptr<RateLimitedActions> rate_limited_actions = std::make_shared<RateLimitedActions>();
} // namespace cqhttp::plugins
