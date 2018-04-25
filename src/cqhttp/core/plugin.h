#pragma once

#include "cqhttp/core/common.h"

#include "cqhttp/core/context.h"

namespace cqhttp {
    struct Plugin {
        virtual ~Plugin() = default;

        virtual void hook_initialize(Context &ctx) { ctx.next(); }
        virtual void hook_enable(Context &ctx) { ctx.next(); }
        virtual void hook_disable(Context &ctx) { ctx.next(); }
        virtual void hook_coolq_start(Context &ctx) { ctx.next(); }
        virtual void hook_coolq_exit(Context &ctx) { ctx.next(); }

        virtual void hook_before_event(EventContext<cq::Event> &ctx) { ctx.next(); }
        virtual void hook_message_event(EventContext<cq::MessageEvent> &ctx) { ctx.next(); }
        virtual void hook_notice_event(EventContext<cq::NoticeEvent> &ctx) { ctx.next(); }
        virtual void hook_request_event(EventContext<cq::RequestEvent> &ctx) { ctx.next(); }
        virtual void hook_after_event(EventContext<cq::Event> &ctx) { ctx.next(); }

        virtual void hook_before_action(ActionContext &ctx) { ctx.next(); }
        virtual void hook_missed_action(ActionContext &ctx) { ctx.next(); }
        virtual void hook_after_action(ActionContext &ctx) { ctx.next(); }
    };
} // namespace cqhttp
