#include "./ws_reverse_service_class.h"
#include "./service_impl_common.h"

using namespace std;
using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;
using WssClient = SimpleWeb::SocketClient<SimpleWeb::WSS>;

template <typename WsClientT>
shared_ptr<WsClientT> WsReverseService::SubServiceBase::init_ws_reverse_client(const string &server_port_path) {
    auto client = make_shared<WsClientT>(server_port_path);
    client->config.header.emplace("User-Agent", CQAPP_USER_AGENT);
    if (!config.access_token.empty()) {
        client->config.header.emplace("Authorization", "Token " + config.access_token);
    }
    client->on_close = [&](shared_ptr<typename WsClientT::Connection> connection,
                           int code, string reason) {
        if (code != 1000) {
            should_reconnect_ = true;
        }
    };
    client->on_error = [&](shared_ptr<typename WsClientT::Connection> connection,
                           const SimpleWeb::error_code &error_code) {
        should_reconnect_ = true;
    };
    return client;
}

void WsReverseService::SubServiceBase::init() {
    Log::d(TAG, u8"初始化反向 WebSocket（" + name() + u8"）");

    auto ws_url = url();

    try {
        if (boost::algorithm::starts_with(ws_url, "ws://")) {
            client_.ws = init_ws_reverse_client<WsClient>(ws_url.substr(strlen("ws://")));
            client_is_wss_ = false;
        } else if (boost::algorithm::starts_with(ws_url, "wss://")) {
            client_.wss = init_ws_reverse_client<WssClient>(ws_url.substr(strlen("wss://")));
            client_is_wss_ = true;
        }
    } catch (...) {
        // in case "init_ws_reverse_client()" failed due to invalid "server_port_path"
        client_is_wss_ = nullopt;
    }

    ServiceBase::init();
}

void WsReverseService::SubServiceBase::finalize() {
    client_.ws = nullptr;
    client_.wss = nullptr;
    client_is_wss_ = nullopt;
    ServiceBase::finalize();
}

void WsReverseService::SubServiceBase::start() {
    if (config.use_ws_reverse) {
        init();

        reconnect_worker_thread_ = thread([&]() {
            try {
                set_reconnect_thread_running_flag(true);
                while (get_reconnect_thread_running_flag()) {
                    if (should_reconnect_) {
                        Log::d(TAG, u8"反向 WebSocket（" + name() + u8"）客户端连接失败或异常断开，将在 "
                               + to_string(config.ws_reverse_reconnect_interval) + u8" 毫秒后尝试重连");
                        should_reconnect_ = false;
                        Sleep(config.ws_reverse_reconnect_interval);
                        stop();
                        start();
                    }

                    if (get_reconnect_thread_running_flag()) {
                        Sleep(500); // wait 500 ms for the next check
                    } else {
                        break;
                    }
                }
            } catch (...) {}

            try {
                remove_reconnect_thread_running_flag();
            } catch (...) {}
        });

        if (client_is_wss_.has_value()) {
            // client successfully initialized
            thread_ = thread([&]() {
                started_ = true;
                try {
                    if (client_is_wss_.value() == false) {
                        client_.ws->start();
                    } else {
                        client_.wss->start();
                    }
                } catch (...) {}
                started_ = false;
            });
            Log::d(TAG, u8"开启 WebSocket 反向客户端（" + name() + u8"）成功，开始连接 " + url());
        }
    }
}

void WsReverseService::SubServiceBase::stop() {
    set_reconnect_thread_running_flag(false, reconnect_worker_thread_.get_id());
    // detach but not join, because we want the thread continue to run until its next check
    reconnect_worker_thread_.detach();

    if (started_) {
        if (client_is_wss_.value() == false) {
            client_.ws->stop();
        } else {
            client_.wss->stop();
        }
        started_ = false;
    }
    if (thread_.joinable()) {
        thread_.join();
    }

    finalize();
}

bool WsReverseService::SubServiceBase::good() const {
    if (config.use_ws_reverse) {
        return initialized_ && started_;
    }
    return ServiceBase::good();
}

string WsReverseService::ApiSubService::url() {
    return config.ws_reverse_api_url;
}

void WsReverseService::ApiSubService::init() {
    SubServiceBase::init();

    if (client_is_wss_.has_value()) {
        if (client_is_wss_.value() == false) {
            client_.ws->on_message = ws_api_on_message<WsClient>;
        } else {
            client_.wss->on_message = ws_api_on_message<WssClient>;
        }
    }
}

string WsReverseService::EventSubService::url() {
    return config.ws_reverse_event_url;
}

void WsReverseService::EventSubService::init() {
    SubServiceBase::init();
}

void WsReverseService::EventSubService::push_event(const json &payload) const {
    if (started_) {
        Log::d(TAG, u8"开始通过 WebSocket 反向客户端上报事件");

        bool succeeded;
        try {
            if (client_is_wss_.value() == false) {
                const auto send_stream = make_shared<WsClient::SendStream>();
                *send_stream << payload.dump();
                // the WsClient class is modified by us ("connection" property made public),
                // so we must maintain the lock manually
                unique_lock<mutex> lock(client_.ws->connection_mutex);
                client_.ws->connection->send(send_stream);
                lock.unlock();
            } else {
                const auto send_stream = make_shared<WssClient::SendStream>();
                *send_stream << payload.dump();
                unique_lock<mutex> lock(client_.wss->connection_mutex);
                client_.wss->connection->send(send_stream);
                lock.unlock();
            }
            succeeded = true;
        } catch (...) {
            succeeded = false;
        }

        Log::d(TAG, u8"通过 WebSocket 反向客户端上报数据到 " + config.ws_reverse_event_url + (succeeded ? u8" 成功" : u8" 失败"));
    }
}
