#pragma once

#include "../service_base_class.h"
#include "../pushable_interface.h"
#include "web_server/client_ws.hpp"
#include "web_server/client_wss.hpp"

class WsReverseService final : public ServiceBase, public IPushable {
public:
    void start() override;
    void stop() override;
    bool good() const override;

    void push_event(const json &payload) const override;

protected:
    void init() override;
    void finalize() override;

private:
    // reverse websocket
    union Client {
        std::shared_ptr<SimpleWeb::SocketClient<SimpleWeb::WS>> ws;
        std::shared_ptr<SimpleWeb::SocketClient<SimpleWeb::WSS>> wss;

        Client() : ws(nullptr) {}
        ~Client() {}
    };

    // reverse websocket api client
    Client api_client_;
    std::optional<bool> api_client_is_wss_;
    std::thread api_thread_;
    bool api_client_started_ = false;
    void start_api_client();
    void stop_api_client();

    // reverse websocket event client
    Client event_client_;
    std::optional<bool> event_client_is_wss_;
    std::thread event_thread_;
    bool event_client_started_ = false;
    void start_event_client();
    void stop_event_client();
};
