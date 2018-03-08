#include "./ini_config_loader.h"

namespace cqhttp::plugins {
    void IniConfigLoader::hook_enable(Context &ctx) {
        ctx.config->raw;
        ctx.next();
    }
} // namespace cqhttp::plugins
