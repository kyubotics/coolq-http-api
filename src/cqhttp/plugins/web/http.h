#pragma once

#include "cqhttp/core/plugin.h"

#include <thread>

#include "./vendor/simple_web/server_http.hpp"

namespace cqhttp::plugins {
    struct Http : Plugin {
        Http() = default;

        void hook_enable(Context &ctx) override;
        void hook_disable(Context &ctx) override;

        void hook_after_event(EventContext<cq::Event> &ctx) override;

    private:
        bool use_http_;
        std::string post_url_;

        std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>> server_;
        std::thread thread_;
    };

    static std::shared_ptr<Http> http = std::make_shared<Http>();
} // namespace cqhttp::plugins
