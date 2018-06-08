#pragma once

#include "cqhttp/core/plugin.h"

#include <thread>

#include "cqhttp/plugins/web/vendor/simple_web/server_http.hpp"

namespace cqhttp::plugins {
    struct Http : Plugin {
        Http() = default;

        void hook_enable(Context &ctx) override;
        void hook_disable(Context &ctx) override;

        void hook_after_event(EventContext<cq::Event> &ctx) override;

        bool good() const override { return !use_http_ || started_; }

    private:
        std::string post_url_{};
        std::string secret_{};
        bool use_http_{};
        std::string access_token_{};
        bool serve_data_files_{};

        std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>> server_;
        std::thread thread_;

        std::atomic_bool started_ = false;

        void init_server();
    };

    static std::shared_ptr<Http> http = std::make_shared<Http>();
} // namespace cqhttp::plugins
