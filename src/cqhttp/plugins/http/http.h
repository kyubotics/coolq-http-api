#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct Http : Plugin {
        void hook_enable(Context &ctx) override {
            logging::debug("http", ctx.config->raw.dump(2));
            ctx.next();
        }

        void hook_after_event(EventContext<cq::Event> &ctx) override;

        void hook_after_action(ActionContext &ctx) override { ctx.next(); }
    };

    static std::shared_ptr<Http> http = std::make_shared<Http>();
} // namespace cqhttp::plugins
