#pragma once

#include "cqhttp/core/plugin.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
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

        class ClientBase {
        public:
            explicit ClientBase(const std::string &url, const std::string &access_token,
                                const unsigned long reconnect_interval, const bool reconnect_on_code_1000)
                : url_(url),
                  access_token_(access_token),
                  reconnect_interval_(reconnect_interval),
                  reconnect_on_code_1000_(reconnect_on_code_1000) {}

            virtual ~ClientBase() = default;

            virtual std::string name() = 0;

            virtual void start();
            virtual void stop();

            virtual bool started() const { return started_; }

        protected:
            virtual void init();
            virtual void connect();
            virtual void disconnect();

            template <typename WsClientT>
            std::shared_ptr<WsClientT> init_ws_reverse_client(const std::string &server_port_path);

            std::string url_;
            std::string access_token_;
            std::chrono::milliseconds reconnect_interval_;
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

            bool should_reconnect_ = false;
            std::thread reconnect_worker_thread_;
            bool reconnect_worker_running_ = false;
            std::mutex mutex_;
            std::condition_variable cv_;

            void notify_should_reconnect() {
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    should_reconnect_ = true;
                }
                cv_.notify_all();
            }

            void notify_reconnect_worker_stop_running() {
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    reconnect_worker_running_ = false;
                }
                cv_.notify_all(); // this will notify the reconnect worker to stop
            }
        };

        class ApiClient final : public ClientBase {
        public:
            using ClientBase::ClientBase;
            std::string name() override { return "API"; }

        protected:
            void init() override;
        };

        std::shared_ptr<ApiClient> api_;

        class EventClient final : public ClientBase {
        public:
            using ClientBase::ClientBase;
            std::string name() override { return "Event"; }

            void push_event(const json &payload) const;
        };

        std::shared_ptr<EventClient> event_;
    };

    static std::shared_ptr<WebSocketReverse> websocket_reverse = std::make_shared<WebSocketReverse>();
} // namespace cqhttp::plugins
