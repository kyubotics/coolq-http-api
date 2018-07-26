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
    shared_ptr<WsClientT> WebSocketReverse::ClientBase::init_ws_reverse_client(const string &server_port_path) {
        auto client = make_shared<WsClientT>(server_port_path);
        client->config.header.emplace("User-Agent", CQHTTP_USER_AGENT);
        client->config.header.emplace("X-Self-ID", to_string(cq::api::get_login_user_id()));
        client->config.header.emplace("X-Client-Role", this->name());
        if (!access_token_.empty()) {
            client->config.header.emplace("Authorization", "Token " + access_token_);
        }
        client->on_close = [&](shared_ptr<typename WsClientT::Connection> connection, const int code, string reason) {
            if (reconnect_on_code_1000_ || code != 1000) {
                notify_should_reconnect();
            }
        };
        client->on_error = [&](shared_ptr<typename WsClientT::Connection>, const SimpleWeb::error_code &) {
            notify_should_reconnect();
        };
        return client;
    }

    void WebSocketReverse::ClientBase::init() {
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

    void WebSocketReverse::ClientBase::connect() {
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
                    notify_should_reconnect();
                }
                started_ = false;
            });
            logging::info_success(TAG, u8"开启反向 WebSocket 客户端（" + name() + u8"）成功，开始连接 " + url_);
        }
    }

    void WebSocketReverse::ClientBase::disconnect() {
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
    }

    void WebSocketReverse::ClientBase::start() {
        init();

        reconnect_worker_thread_ = thread([&]() {
            {
                unique_lock<mutex> lock(mutex_);
                reconnect_worker_running_ = true;
            }
            for (;;) {
                try {
                    auto should_reconn = false;
                    auto should_stop = false;

                    {
                        unique_lock<mutex> lock(mutex_);
                        cv_.wait(lock, [&] {
                            should_reconn = should_reconnect_;
                            should_stop = !reconnect_worker_running_;
                            return should_reconn || should_stop; // stop waiting only if we should reconnect or stop
                        });
                    }

                    if (should_stop) {
                        break;
                    }

                    if (should_reconn) {
                        with_unique_lock(mutex_, [&] { should_reconnect_ = false; });

                        logging::warning(TAG,
                                         u8"反向 WebSocket（" + name() + u8"）客户端连接失败或异常断开，将在 "
                                             + to_string(reconnect_interval_.count()) + u8" 毫秒后尝试重连");

                        // wait for reconnect_interval_, if during the interval we are notified to stop, we stop
                        if (unique_lock<mutex> lock(mutex_);
                            cv_.wait_for(lock, reconnect_interval_, [&] { return !reconnect_worker_running_; })) {
                            // we are notified to stop, so break the for loop
                            break;
                        }

                        // reconnect_interval_ passed, retry to connect
                        disconnect();
                        connect();
                    }
                } catch (...) {
                }
            }
        });

        connect();
    }

    void WebSocketReverse::ClientBase::stop() {
        notify_reconnect_worker_stop_running();
        if (reconnect_worker_thread_.joinable()) {
            reconnect_worker_thread_.join();
        }

        disconnect();
        client_.ws = nullptr;
        client_.wss = nullptr;
        client_is_wss_ = nullopt;
    }

    void WebSocketReverse::ApiClient::init() {
        ClientBase::init();

        if (client_is_wss_.has_value()) {
            if (client_is_wss_.value() == false) {
                client_.ws->on_message = ws_api_on_message<WsClient>;
            } else {
                client_.wss->on_message = ws_api_on_message<WssClient>;
            }
        }
    }

    void WebSocketReverse::EventClient::push_event(const json &payload) const {
        if (started_) {
            logging::debug(TAG, u8"开始通过反向 WebSocket 客户端上报事件");

            bool succeeded;
            try {
                if (client_is_wss_.value() == false) {
                    const auto out_message = make_shared<WsClient::OutMessage>();
                    *out_message << payload.dump();
                    // the WsClient class is modified by us ("connection" property made public),
                    // so we must maintain the lock manually
                    unique_lock<mutex> lock(client_.ws->connection_mutex);
                    client_.ws->connection->send(out_message);
                    lock.unlock();
                } else {
                    const auto out_message = make_shared<WssClient::OutMessage>();
                    *out_message << payload.dump();
                    unique_lock<mutex> lock(client_.wss->connection_mutex);
                    client_.wss->connection->send(out_message);
                    lock.unlock();
                }
                succeeded = true;
            } catch (...) {
                succeeded = false;
            }

            if (succeeded) {
                logging::info_success(TAG, u8"通过反向 WebSocket 客户端上报数据到 " + url_ + u8" 成功");
            } else {
                logging::warning(TAG, u8"通过反向 WebSocket 客户端上报数据到 " + url_ + u8" 失败");
            }
        }
    }
} // namespace cqhttp::plugins
