#pragma once

#include "cqhttp/core/plugin.h"

#include <thread>

#include "cqhttp/plugins/web/vendor/simple_web/server_ws.hpp"

namespace cqhttp::plugins {
    struct WebSocket : Plugin {
        WebSocket() = default;
        std::string name() const override { return "websocket"; }

        void hook_enable(Context &ctx) override;
        void hook_disable(Context &ctx) override;

        void hook_after_event(EventContext<cq::Event> &ctx) override;

        bool good() const override { return !use_ws_ || started_; }

    private:
        bool use_ws_{};
        std::string access_token_{};

        std::shared_ptr<SimpleWeb::SocketServer<SimpleWeb::WS>> server_;
        std::thread thread_;

        std::atomic_bool started_ = false;

        void init_server();
    };

    static std::shared_ptr<WebSocket> websocket = std::make_shared<WebSocket>();
} // namespace cqhttp::plugins
