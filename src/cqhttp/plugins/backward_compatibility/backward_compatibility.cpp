#include "./backward_compatibility.h"

#include <ctime>

using namespace std;

namespace cqhttp::plugins {
    void BackwardCompatibility::hook_enable(Context &ctx) {
        enabled_ = ctx.config->get_bool("enable_backward_compatibility", true);
    }

    void BackwardCompatibility::hook_message_event(EventContext<cq::MessageEvent> &ctx) {
        if (enabled_) {
            ctx.data["time"] = time(nullptr);
        }

        ctx.next();
    }

    void BackwardCompatibility::hook_notice_event(EventContext<cq::NoticeEvent> &ctx) {
        if (enabled_) {
            ctx.data["post_type"] = "event";
            ctx.data["event"] = ctx.data["notice_type"];
            ctx.data.erase("notice_type");
        }

        ctx.next();
    }

    void BackwardCompatibility::hook_request_event(EventContext<cq::RequestEvent> &ctx) {
        if (enabled_) {
            if (ctx.event.request_type == cq::request::FRIEND || ctx.event.request_type == cq::request::GROUP) {
                ctx.data["message"] = ctx.data["comment"];
                ctx.data.erase("comment");
            }
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
