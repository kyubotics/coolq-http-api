#include "./post_message_formatter.h"

namespace cqhttp::plugins {
    void PostMessageFormatter::hook_enable(Context &ctx) {
        post_message_format_ = ctx.config->get_string("post_message_format", "string");
        if (post_message_format_ != "string" && post_message_format_ != "array") {
            logging::warning(u8"消息格式",
                             u8"上报消息格式 " + post_message_format_ + u8" 不支持，将使用默认格式：string");
            post_message_format_ = "string";
        }

        ctx.next();
    }

    void PostMessageFormatter::hook_after_event(EventContext<cq::Event> &ctx) {
        if (ctx.event.type == cq::event::MESSAGE && post_message_format_ == "string") {
            ctx.data["message"] = std::to_string(ctx.data["message"].get<cq::Message>());
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
