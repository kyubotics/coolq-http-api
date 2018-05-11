#pragma once

#include "cqhttp/core/common.h"

#include "cqhttp/core/application.h"
#include "cqhttp/core/plugin.h"

namespace cqhttp {
    extern Application app;

    /**
     * Initialize the module.
     * This will assign the event handlers of "cqhttp" module to "cqsdk" module.
     */
    void init();

    inline void use(const std::shared_ptr<Plugin> plugin) { app.plugins_.push_back(plugin); }
} // namespace cqhttp
