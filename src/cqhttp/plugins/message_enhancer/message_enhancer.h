#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct MessageEnhancer : Plugin {
        std::string name() const override { return "message_enhancer"; }
        void hook_message_event(EventContext<cq::MessageEvent> &ctx) override;
        void hook_before_action(ActionContext &ctx) override;
    };

    static std::shared_ptr<MessageEnhancer> message_enhancer = std::make_shared<MessageEnhancer>();
} // namespace cqhttp::plugins
