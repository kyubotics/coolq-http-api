#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct JsonConfigLoader : Plugin {
        void hook_enable(Context &ctx) override;
    };

    static std::shared_ptr<JsonConfigLoader> json_config_loader = std::make_shared<JsonConfigLoader>();
} // namespace cqhttp::plugins
