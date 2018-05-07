#pragma once

#include "cqhttp/core/plugin.h"

#include "cqhttp/plugins/event_filter/filter.h"

namespace cqhttp::plugins {
    struct EventFilter : Plugin {
        void hook_enable(Context &ctx) override;
        void hook_disable(Context &ctx) override;
        void hook_after_event(EventContext<cq::Event> &ctx) override;

    private:
        std::shared_ptr<Filter> filter_;
    };

    static std::shared_ptr<EventFilter> event_filter = std::make_shared<EventFilter>();
} // namespace cqhttp::plugins
