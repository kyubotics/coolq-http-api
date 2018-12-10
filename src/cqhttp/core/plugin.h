#pragma once

#include "cqhttp/core/common.h"

#include "cqhttp/core/context.h"
#include "cqhttp/core/event.h"

namespace cqhttp {
    /**
     * Lifecycle:
     *
     * +-------------------------------------------------+
     * | hook_initialize                                 |
     * |        +                                        |
     * |        | enabled by user                        |
     * |        v                                        |
     * | hook_coolq_start                                |
     * |   hook_enable+-----------+                      |
     * |        +                 | disabled by user     |
     * |        | coolq closed    v                      |
     * |        v            hook_disable                |
     * |   hook_disable           +                      |
     * |  hook_coolq_exit         | coolq closed         |
     * |                          v                      |
     * |                    hook_coolq_exit              |
     * +-------------------------------------------------+
     *
     * The lifecycle here is NOT quite the same as one in "cqsdk" module,
     * because "cqhttp::Application" class will generate a fake "disable" event
     * if necessary at exit, which means "hook_enable" and "hook_disable" methods
     * are guaranteed to be called as a pair, so plugins can safely start
     * their services in "hook_enable" and stop in "hook_disable" and have no need
     * to care about the "coolq_*" events.
     */
    struct Plugin {
        Plugin() = default;
        virtual ~Plugin() = default;

        virtual std::string name() const = 0;

        virtual void hook_initialize(Context &ctx) { ctx.next(); }
        virtual void hook_enable(Context &ctx) { ctx.next(); }
        virtual void hook_disable(Context &ctx) { ctx.next(); }
        virtual void hook_coolq_start(Context &ctx) { ctx.next(); }
        virtual void hook_coolq_exit(Context &ctx) { ctx.next(); }

        virtual void hook_before_event(EventContext<cq::Event> &ctx) { ctx.next(); }
        virtual void hook_message_event(EventContext<cq::MessageEvent> &ctx) { ctx.next(); }
        virtual void hook_notice_event(EventContext<cq::NoticeEvent> &ctx) { ctx.next(); }
        virtual void hook_request_event(EventContext<cq::RequestEvent> &ctx) { ctx.next(); }
        virtual void hook_meta_event(EventContext<cqhttp::MetaEvent> &ctx) { ctx.next(); }
        virtual void hook_after_event(EventContext<cq::Event> &ctx) { ctx.next(); }

        virtual void hook_before_action(ActionContext &ctx) { ctx.next(); }
        virtual void hook_missed_action(ActionContext &ctx) { ctx.next(); }
        virtual void hook_after_action(ActionContext &ctx) { ctx.next(); }

        virtual bool good() const { return true; }
    };
} // namespace cqhttp
