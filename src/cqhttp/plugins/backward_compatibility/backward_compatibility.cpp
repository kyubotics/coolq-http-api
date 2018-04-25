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

            if (ctx.event.message_type == cq::message::GROUP) {
                auto &e = static_cast<const cq::GroupMessageEvent &>(ctx.event);
                if (e.is_anonymous()) {
                    ctx.data["anonymous"] = e.anonymous.name;
                    ctx.data["anonymous_flag"] = e.anonymous.flag;
                } else {
                    ctx.data["anonymous"] = "";
                    ctx.data["anonymous_flag"] = "";
                }
            }
        }

        ctx.next();
    }

    void BackwardCompatibility::hook_notice_event(EventContext<cq::NoticeEvent> &ctx) {
        if (enabled_) {
            ctx.data["post_type"] = "event";
            ctx.data["event"] = ctx.event.notice_type;
            ctx.data.erase("notice_type");
        }

        ctx.next();
    }

    void BackwardCompatibility::hook_request_event(EventContext<cq::RequestEvent> &ctx) {
        if (enabled_) {
            if (ctx.event.request_type == cq::request::FRIEND || ctx.event.request_type == cq::request::GROUP) {
                ctx.data["message"] = ctx.event.comment;
                ctx.data.erase("comment");
            }
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
