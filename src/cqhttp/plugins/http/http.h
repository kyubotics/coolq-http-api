#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct Http : Plugin {
        Http() = default;

        void hook_enable(Context &ctx) override;

        void hook_after_event(EventContext<cq::Event> &ctx) override;

    private:
        bool use_http_;
        std::string post_url_;
    };

    static std::shared_ptr<Http> http = std::make_shared<Http>();
} // namespace cqhttp::plugins
