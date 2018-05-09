#include "./websocket_reverse.h"

#include "cqhttp/plugins/web/ws_common.h"
#include "cqhttp/utils/mutex.h"

using namespace std;
using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;
using WssClient = SimpleWeb::SocketClient<SimpleWeb::WSS>;

namespace cqhttp::plugins {
    static const auto TAG = "反向WS";

    using utils::mutex::with_unique_lock;

    template <typename WsClientT>
    shared_ptr<WsClientT> WebSocketReverse::EndpointBase::init_ws_reverse_client(const string &server_port_path) {
        auto client = make_shared<WsClientT>(server_port_path);
        client->config.header.emplace("User-Agent", CQHTTP_USER_AGENT);
        if (!access_token_.empty()) {
            client->config.header.emplace("Authorization", "Token " + access_token_);
        }
        client->on_close = [&](shared_ptr<typename WsClientT::Connection> connection, const int code, string reason) {
            if (reconnect_on_code_1000_ || code != 1000) {
                with_unique_lock(should_reconnect_mutex_, [&]() { should_reconnect_ = true; });
            }
        };
        client->on_error = [&](shared_ptr<typename WsClientT::Connection> connection,
                               const SimpleWeb::error_code &error_code) {
            with_unique_lock(should_reconnect_mutex_, [&]() { should_reconnect_ = true; });
        };
        return client;
    }

    void WebSocketReverse::EndpointBase::init() {
        try {
            if (boost::algorithm::starts_with(url_, "ws://")) {
                client_.ws = init_ws_reverse_client<WsClient>(url_.substr(strlen("ws://")));
                client_is_wss_ = false;
            } else if (boost::algorithm::starts_with(url_, "wss://")) {
                client_.wss = init_ws_reverse_client<WssClient>(url_.substr(strlen("wss://")));
                client_is_wss_ = true;
            }
        } catch (...) {
            // in case "init_ws_reverse_client()" failed due to invalid "server_port_path"
            client_is_wss_ = nullopt;
        }
    }

    void WebSocketReverse::EndpointBase::start() {
        init();

        reconnect_worker_thread_ = thread([&]() {
            try {
                set_reconnect_worker_running(true);
                while (is_reconnect_worker_running()) {
                    auto should_reconn = false;
                    with_unique_lock(should_reconnect_mutex_, [&]() {
                        should_reconn = should_reconnect_;
                        should_reconnect_ = false;
                    });
                    if (should_reconn) {
                        logging::info(TAG,
                                      u8"反向 WebSocket（" + name() + u8"）客户端连接失败或异常断开，将在 "
                                          + to_string(reconnect_interval_) + u8" 毫秒后尝试重连");
                        Sleep(reconnect_interval_);
                        stop();
                        start();
                    }

                    if (is_reconnect_worker_running()) {
                        Sleep(500); // wait 500 ms for the next check
                    } else {
                        break;
                    }
                }
            } catch (...) {
            }
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
                } catch (...) {
                }
                started_ = false;
            });
            logging::info_success(TAG, u8"开启 WebSocket 反向客户端（" + name() + u8"）成功，开始连接 " + url_);
        }
    }

    void WebSocketReverse::EndpointBase::stop() {
        set_reconnect_worker_running(false); // this will notify the reconnect worker to stop
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

        client_.ws = nullptr;
        client_.wss = nullptr;
        client_is_wss_ = nullopt;
    }

    void WebSocketReverse::ApiEndpoint::init() {
        EndpointBase::init();

        if (client_is_wss_.has_value()) {
            if (client_is_wss_.value() == false) {
                client_.ws->on_message = ws_api_on_message<WsClient>;
            } else {
                client_.wss->on_message = ws_api_on_message<WssClient>;
            }
        }
    }

    void WebSocketReverse::EventEndpoint::push_event(const json &payload) const {
        if (started_) {
            logging::debug(TAG, u8"开始通过 WebSocket 反向客户端上报事件");

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

            if (succeeded) {
                logging::info_success(TAG, u8"通过 WebSocket 反向客户端上报数据到 " + url_ + u8" 成功");
            } else {
                logging::info(TAG, u8"通过 WebSocket 反向客户端上报数据到 " + url_ + u8" 失败");
            }
        }
    }
} // namespace cqhttp::plugins
