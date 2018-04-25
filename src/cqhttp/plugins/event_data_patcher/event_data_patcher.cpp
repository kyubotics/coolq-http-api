#include "./event_data_patcher.h"

#include <ctime>

namespace cqhttp::plugins {
    void EventDataPatcher::hook_after_event(EventContext<cq::Event> &ctx) {
        ctx.data["self_id"] = cq::api::get_login_user_id();
        if (ctx.data.find("time") == ctx.data.end()) {
            ctx.data["time"] = time(nullptr);
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
