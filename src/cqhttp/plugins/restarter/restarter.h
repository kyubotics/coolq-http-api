#pragma once

#include "cqhttp/core/plugin.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace cqhttp::plugins {
    struct Restarter : Plugin {
        std::string name() const override { return "restarter"; }
        void hook_initialize(Context &ctx) override;
        void hook_coolq_exit(Context &ctx) override;
        void hook_missed_action(ActionContext &ctx) override;
        bool good() const override { return restart_worker_running_; }

    private:
        std::atomic_bool should_restart_ = false;
        std::chrono::milliseconds restart_delay_;
        std::thread restart_worker_thread_;
        std::atomic_bool restart_worker_running_ = false;

        std::mutex mutex_;
        std::condition_variable cv_;
    };

    static std::shared_ptr<Restarter> restarter = std::make_shared<Restarter>();
} // namespace cqhttp::plugins
