#include "./ws_reverse_service_class.h"
#include "./service_impl_common.h"

using namespace std;
using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;
using WssClient = SimpleWeb::SocketClient<SimpleWeb::WSS>;

/**
 * \brief Create a reverse websocket client instance.
 * \tparam WsClientT WsClient or WssClient (WebSocket SSL)
 * \param server_port_path destination to connect
 * \return the newly created client instance (as shared_ptr)
 */
template <typename WsClientT>
static shared_ptr<WsClientT> init_ws_reverse_client(const string &server_port_path) {
    auto client = make_shared<WsClientT>(server_port_path);
    client->config.header.emplace("User-Agent", CQAPP_USER_AGENT);
    if (!config.access_token.empty()) {
        client->config.header.emplace("Authorization", "Token " + config.access_token);
    }
    return client;
}

void WsReverseService::init() {
    Log::d(TAG, u8"初始化反向 WebSocket");

    try {
        if (boost::algorithm::starts_with(config.ws_reverse_api_url, "ws://")) {
            api_client_.ws = init_ws_reverse_client<WsClient>(
                config.ws_reverse_api_url.substr(strlen("ws://")));
            api_client_.ws->on_message = ws_api_on_message<WsClient>;
            api_client_is_wss_ = false;
        } else if (boost::algorithm::starts_with(config.ws_reverse_api_url, "wss://")) {
            api_client_.wss = init_ws_reverse_client<WssClient>(
                config.ws_reverse_api_url.substr(strlen("wss://")));
            api_client_.wss->on_message = ws_api_on_message<WssClient>;
            api_client_is_wss_ = true;
        }
    } catch (...) {
        // in case "init_ws_reverse_client()" failed due to invalid "server_port_path"
        api_client_is_wss_ = nullopt;
    }

    try {
        if (boost::algorithm::starts_with(config.ws_reverse_event_url, "ws://")) {
            event_client_.ws = init_ws_reverse_client<WsClient>(
                config.ws_reverse_event_url.substr(strlen("ws://")));
            event_client_is_wss_ = false;
        } else if (boost::algorithm::starts_with(config.ws_reverse_event_url, "wss://")) {
            event_client_.wss = init_ws_reverse_client<WssClient>(
                config.ws_reverse_event_url.substr(strlen("wss://")));
            event_client_is_wss_ = true;
        }
    } catch (...) {
        // in case "init_ws_reverse_client()" failed due to invalid "server_port_path"
        event_client_is_wss_ = nullopt;
    }

    ServiceBase::init();
}

void WsReverseService::finalize() {
    api_client_.ws = nullptr;
    api_client_.wss = nullptr;
    api_client_is_wss_ = nullopt;
    api_client_started_ = false;

    event_client_.ws = nullptr;
    event_client_.wss = nullptr;
    event_client_is_wss_ = nullopt;
    event_client_started_ = false;

    ServiceBase::finalize();
}

void WsReverseService::start_api_client() {
    if (api_client_is_wss_.has_value()) {
        // client successfully initialized
        api_thread_ = thread([&]() {
            api_client_started_ = true;
            try {
                if (api_client_is_wss_.value() == false) {
                    api_client_.ws->start();
                } else {
                    api_client_.wss->start();
                }
            } catch (...) {}
            api_client_started_ = false;
        });
        Log::d(TAG, u8"开启 API WebSocket 反向客户端成功，开始连接 " + config.ws_reverse_api_url);
    }
}

void WsReverseService::stop_api_client() {
    if (api_client_started_) {
        if (api_client_is_wss_.value() == false) {
            api_client_.ws->stop();
        } else {
            api_client_.wss->stop();
        }
        api_client_started_ = false;
    }
    if (api_thread_.joinable()) {
        api_thread_.join();
    }
}

void WsReverseService::start_event_client() {
    if (event_client_is_wss_.has_value()) {
        // client successfully initialized
        event_thread_ = thread([&]() {
            event_client_started_ = true;
            try {
                if (event_client_is_wss_.value() == false) {
                    event_client_.ws->start();
                } else {
                    event_client_.wss->start();
                }
            } catch (...) {}
            event_client_started_ = false;
        });
        Log::d(TAG, u8"开启 Event WebSocket 反向客户端成功，开始连接 " + config.ws_reverse_event_url);
    }
}

void WsReverseService::stop_event_client() {
    if (event_client_started_) {
        if (event_client_is_wss_.value() == false) {
            event_client_.ws->stop();
        } else {
            event_client_.wss->stop();
        }
        event_client_started_ = false;
    }
    if (event_thread_.joinable()) {
        event_thread_.join();
    }
}

void WsReverseService::start() {
    if (config.use_ws_reverse) {
        init();
        start_api_client();
        start_event_client();
        started_ = api_client_started_ && event_client_started_;
    }
}

void WsReverseService::stop() {
    stop_api_client();
    stop_event_client();
    started_ = api_client_started_ && event_client_started_;
    finalize();
}

bool WsReverseService::good() const {
    if (config.use_ws_reverse) {
        return initialized_ && api_client_started_ && event_client_started_;
    }
    return ServiceBase::good();
}

void WsReverseService::push_event(const json &payload) const {
    if (event_client_started_) {
        Log::d(TAG, u8"开始通过 WebSocket 反向客户端上报事件");

        bool succeeded;
        try {
            if (event_client_is_wss_.value() == false) {
                const auto send_stream = make_shared<WsClient::SendStream>();
                *send_stream << payload.dump();
                // the WsClient class is modified by us ("connection" property made public),
                // so we must maintain the lock manually
                unique_lock<mutex> lock(event_client_.ws->connection_mutex);
                event_client_.ws->connection->send(send_stream);
                lock.unlock();
            } else {
                const auto send_stream = make_shared<WssClient::SendStream>();
                *send_stream << payload.dump();
                unique_lock<mutex> lock(event_client_.wss->connection_mutex);
                event_client_.wss->connection->send(send_stream);
                lock.unlock();
            }
            succeeded = true;
        } catch (...) {
            succeeded = false;
        }

        Log::d(TAG, u8"通过 WebSocket 反向客户端上报数据到 " + config.ws_reverse_event_url + (succeeded ? u8" 成功" : u8" 失败"));
    }
}
