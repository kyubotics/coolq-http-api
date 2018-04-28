#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct PostMessageFormatter : Plugin {
        void hook_enable(Context &ctx) override;
        void hook_after_event(EventContext<cq::Event> &ctx) override;

    private:
        std::string post_message_format_;
    };

    static std::shared_ptr<PostMessageFormatter> post_message_formatter = std::make_shared<PostMessageFormatter>();
} // namespace cqhttp::plugins
