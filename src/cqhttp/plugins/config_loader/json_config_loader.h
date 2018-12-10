#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct JsonConfigLoader : Plugin {
        std::string name() const override { return "json_config_loader"; }
        void hook_enable(Context &ctx) override;
    };

    static std::shared_ptr<JsonConfigLoader> json_config_loader = std::make_shared<JsonConfigLoader>();
} // namespace cqhttp::plugins
