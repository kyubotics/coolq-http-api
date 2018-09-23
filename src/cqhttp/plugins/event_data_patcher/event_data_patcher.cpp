#include "./event_data_patcher.h"

#include <ctime>

namespace cqhttp::plugins {
    void EventDataPatcher::hook_after_event(EventContext<cq::Event> &ctx) {
        ctx.data["self_id"] = cq::api::get_login_user_id();
        if (ctx.data.find("time") == ctx.data.end()) {
            ctx.data["time"] = time(nullptr);
        }

        if (ctx.data["post_type"] == "message") {
            json sender = {
                {"user_id", ctx.data["user_id"]},
            };

            if (ctx.data["message_type"] == "group") {
                try {
                    auto info = cq::api::get_group_member_info(ctx.data["group_id"], ctx.data["user_id"]);
                    sender.update({
                        {"nickname", info.nickname},
                        {"card", info.card},
                        {"sex", info.sex},
                        {"age", info.age},
                    });
                } catch (const cq::exception::ApiError &) {
                }
            }

            if (sender.count("nickname") == 0) {
                try {
                    auto info = cq::api::get_stranger_info(ctx.data["user_id"]);
                    sender.update({
                        {"nickname", info.nickname},
                        {"sex", info.sex},
                        {"age", info.age},
                    });
                } catch (const cq::exception::ApiError &) {
                }
            }

            ctx.data.emplace("sender", sender);
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
