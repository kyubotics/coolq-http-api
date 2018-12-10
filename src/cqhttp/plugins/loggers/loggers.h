#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct Loggers : Plugin {
        std::string name() const override { return "loggers"; }
        void hook_enable(Context &ctx) override;
        void hook_disable(Context &ctx) override;
        void hook_missed_action(ActionContext &ctx) override;

    private:
        size_t max_file_size_{};
        size_t max_files_{};

        void create_file_logger();
    };

    static std::shared_ptr<Loggers> loggers = std::make_shared<Loggers>();
} // namespace cqhttp::plugins
