#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct DefaultConfigGenerator : Plugin {
        std::string name() const override { return "default_config_generator"; }
        void hook_enable(Context &ctx) override;
    };

    static std::shared_ptr<DefaultConfigGenerator> default_config_generator =
        std::make_shared<DefaultConfigGenerator>();
} // namespace cqhttp::plugins
