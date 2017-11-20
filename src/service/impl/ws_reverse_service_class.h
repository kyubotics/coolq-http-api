#pragma once

#include "../service_base_class.h"
#include "../pushable_interface.h"
#include "web_server/client_ws.hpp"
#include "web_server/client_wss.hpp"

class WsReverseService final : public ServiceBase, public IPushable {
public:
    void start() override {
        api_.start();
        event_.start();
    }

    void stop() override {
        api_.stop();
        event_.stop();
    }

    bool initialized() const override {
        return api_.initialized() && event_.initialized();
    }

    bool started() const override {
        return api_.started() && event_.started();
    }

    bool good() const override {
        return api_.good() && event_.good();
    }

    void push_event(const json &payload) const override {
        event_.push_event(payload);
    }

private:
    class SubServiceBase : public ServiceBase {
    public:
        virtual std::string name() = 0;
        virtual std::string url() = 0;

        void start() override;
        void stop() override;
        bool good() const override;

    protected:
        void init() override;
        void finalize() override;

        union Client {
            std::shared_ptr<SimpleWeb::SocketClient<SimpleWeb::WS>> ws;
            std::shared_ptr<SimpleWeb::SocketClient<SimpleWeb::WSS>> wss;

            Client() : ws(nullptr) {}
            ~Client() {}
        };

        Client client_;
        std::optional<bool> client_is_wss_;
        std::thread thread_;

    private:
        template <typename WsClientT>
        std::shared_ptr<WsClientT> init_ws_reverse_client(const std::string &server_port_path);

        bool should_reconnect_ = false;
        std::unordered_map<std::thread::id, bool> reconnect_thread_running_flag_map_;
        std::mutex reconnect_thread_running_flag_map_mutex_;
        std::thread reconnect_worker_thread_;

        bool get_reconnect_thread_running_flag(const std::thread::id tid = std::this_thread::get_id()) {
            std::unique_lock<std::mutex> lock(reconnect_thread_running_flag_map_mutex_);
            if (const auto &it = reconnect_thread_running_flag_map_.find(tid);
                it != reconnect_thread_running_flag_map_.end()) {
                return it->second;
            }
            return false;
        }

        void set_reconnect_thread_running_flag(const bool flag,
                                               const std::thread::id tid = std::this_thread::get_id()) {
            std::unique_lock<std::mutex> lock(reconnect_thread_running_flag_map_mutex_);
            reconnect_thread_running_flag_map_[tid] = flag;
        }

        void remove_reconnect_thread_running_flag(const std::thread::id tid = std::this_thread::get_id()) {
            std::unique_lock<std::mutex> lock(reconnect_thread_running_flag_map_mutex_);
            reconnect_thread_running_flag_map_.erase(tid);
        }
    };

    class ApiSubService final : public SubServiceBase {
    public:
        std::string name() override {
            return "API";
        }

        std::string url() override;

    protected:
        void init() override;
    } api_;

    class EventSubService final : public SubServiceBase, public IPushable {
    public:
        std::string name() override {
            return "Event";
        }

        std::string url() override;

        void push_event(const json &payload) const override;

    protected:
        void init() override;
    } event_;
};
