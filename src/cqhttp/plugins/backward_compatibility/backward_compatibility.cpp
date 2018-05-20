#include "./backward_compatibility.h"

using namespace std;

namespace cqhttp::plugins {
    void BackwardCompatibility::hook_enable(Context &ctx) {
        enabled_ = ctx.config->get_bool("enable_backward_compatibility", true);
        ctx.next();
    }

    void BackwardCompatibility::hook_after_event(EventContext<cq::Event> &ctx) {
        if (enabled_) {
            switch (ctx.event.type) {
            case cq::event::MESSAGE: {
                auto &e = static_cast<const cq::MessageEvent &>(ctx.event);
                if (e.message_type == cq::message::GROUP) {
                    auto &gme = static_cast<const cq::GroupMessageEvent &>(ctx.event);
                    if (gme.is_anonymous()) {
                        ctx.data["anonymous"] = gme.anonymous.name;
                        ctx.data["anonymous_flag"] = gme.anonymous.flag;
                    } else {
                        ctx.data["anonymous"] = "";
                        ctx.data["anonymous_flag"] = "";
                    }
                }
                break;
            }
            case cq::event::NOTICE: {
                auto &e = static_cast<const cq::NoticeEvent &>(ctx.event);
                ctx.data["post_type"] = "event";
                ctx.data["event"] = e.notice_type;
                ctx.data.erase("notice_type");
                break;
            }
            case cq::event::REQUEST: {
                auto &e = static_cast<const cq::RequestEvent &>(ctx.event);
                ctx.data["message"] = e.comment;
                ctx.data.erase("comment");
                break;
            }
            default:
                break;
            }
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
