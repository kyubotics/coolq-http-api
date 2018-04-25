#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct BackwardCompatibility : Plugin {
        BackwardCompatibility() = default;

        void hook_enable(Context &ctx) override;

        void hook_message_event(EventContext<cq::MessageEvent> &ctx) override;
        void hook_notice_event(EventContext<cq::NoticeEvent> &ctx) override;
        void hook_request_event(EventContext<cq::RequestEvent> &ctx) override;

    private:
        bool enabled_;
    };

    static std::shared_ptr<BackwardCompatibility> backward_compatibility = std::make_shared<BackwardCompatibility>();
} // namespace cqhttp::plugins
