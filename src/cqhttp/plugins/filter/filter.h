#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct Filter : Plugin {
        void hook_message_event(EventContext<cq::MessageEvent> &ctx) override {
            logging::debug("filter", "111");
            ctx.next();
        }
    };

    static std::shared_ptr<Filter> filter = std::make_shared<Filter>();
} // namespace cqhttp::plugins
