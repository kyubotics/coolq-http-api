#pragma once

#include "cqhttp/core/plugin.h"

#include <thread>

#include "cqhttp/plugins/web/vendor/simple_web/client_ws.hpp"
#include "cqhttp/plugins/web/vendor/simple_web/client_wss.hpp"

namespace cqhttp::plugins {
    struct WebSocketReverse : Plugin {
        WebSocketReverse() = default;

        void hook_enable(Context &ctx) override;
        void hook_disable(Context &ctx) override;
        void hook_after_event(EventContext<cq::Event> &ctx) override;

        bool good() const override { return (!api_ || api_->started()) && (!event_ || event_->started()); }

    private:
        bool use_ws_reverse_;

        class EndpointBase {
        public:
            explicit EndpointBase(const std::string &url, const std::string &access_token,
                                  const unsigned long reconnect_interval, const bool reconnect_on_code_1000)
                : url_(url),
                  access_token_(access_token),
                  reconnect_interval_(reconnect_interval),
                  reconnect_on_code_1000_(reconnect_on_code_1000) {}

            virtual ~EndpointBase() = default;

            virtual std::string name() = 0;

            virtual void start();
            virtual void stop();

            virtual bool started() const { return started_; }

        protected:
            virtual void init();

            template <typename WsClientT>
            std::shared_ptr<WsClientT> init_ws_reverse_client(const std::string &server_port_path);

            std::string url_;
            std::string access_token_;
            unsigned long reconnect_interval_;
            bool reconnect_on_code_1000_;

            std::atomic_bool started_ = false;

            union Client {
                std::shared_ptr<SimpleWeb::SocketClient<SimpleWeb::WS>> ws;
                std::shared_ptr<SimpleWeb::SocketClient<SimpleWeb::WSS>> wss;

                Client() : ws(nullptr) {}
                ~Client() {}
            };

            Client client_;
            std::optional<bool> client_is_wss_;
            std::thread thread_;

            std::atomic_bool should_reconnect_ = false;
            std::thread reconnect_worker_thread_;
            std::atomic_bool reconnect_worker_running_ = false;
        };

        class ApiEndpoint final : public EndpointBase {
        public:
            using EndpointBase::EndpointBase;
            std::string name() override { return "API"; }

        protected:
            void init() override;
        };

        std::shared_ptr<ApiEndpoint> api_;

        class EventEndpoint final : public EndpointBase {
        public:
            using EndpointBase::EndpointBase;
            std::string name() override { return "Event"; }

            void push_event(const json &payload) const;
        };

        std::shared_ptr<EventEndpoint> event_;
    };

    static std::shared_ptr<WebSocketReverse> websocket_reverse = std::make_shared<WebSocketReverse>();
} // namespace cqhttp::plugins
