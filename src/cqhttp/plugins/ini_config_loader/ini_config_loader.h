#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct IniConfigLoader : Plugin {
        void hook_enable(Context &ctx) override;
    };
} // namespace cqhttp::plugins
