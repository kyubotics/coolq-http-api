#pragma once

#include "cqhttp/core/plugin.h"

#include <thread>

namespace cqhttp::plugins {
    struct Restarter : Plugin {
        void hook_initialize(Context &ctx) override;
        void hook_coolq_exit(Context &ctx) override;
        void hook_missed_action(ActionContext &ctx) override;

    private:
        bool should_restart_ = false;
        unsigned long restart_delay_ = 0;
        std::thread restart_worker_thread_;
        bool restart_worker_running_ = false;
    };

    static std::shared_ptr<Restarter> restarter = std::make_shared<Restarter>();
} // namespace cqhttp::plugins
