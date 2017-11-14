#include "./server_impl_common.h"

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

void ApiServer::init_ws_reverse() {
	Log::d(TAG, u8"初始化反向 WebSocket");

	try {
		if (boost::algorithm::starts_with(config.ws_reverse_api_url, "ws://")) {
			ws_reverse_api_client_.ws = init_ws_reverse_client<WsClient>(
				config.ws_reverse_api_url.substr(strlen("ws://")));
			ws_reverse_api_client_.ws->on_message = ws_api_on_message<WsClient>;
			ws_reverse_api_client_is_wss_ = false;
		} else if (boost::algorithm::starts_with(config.ws_reverse_api_url, "wss://")) {
			ws_reverse_api_client_.wss = init_ws_reverse_client<WssClient>(
				config.ws_reverse_api_url.substr(strlen("wss://")));
			ws_reverse_api_client_.wss->on_message = ws_api_on_message<WssClient>;
			ws_reverse_api_client_is_wss_ = true;
		}
	} catch (...) {
		// in case "init_ws_reverse_client()" failed due to invalid "server_port_path"
		ws_reverse_api_client_is_wss_ = nullopt;
	}

	try {
		if (boost::algorithm::starts_with(config.ws_reverse_event_url, "ws://")) {
			ws_reverse_event_client_.ws = init_ws_reverse_client<WsClient>(
				config.ws_reverse_event_url.substr(strlen("ws://")));
			ws_reverse_event_client_is_wss_ = false;
		} else if (boost::algorithm::starts_with(config.ws_reverse_event_url, "wss://")) {
			ws_reverse_event_client_.wss = init_ws_reverse_client<WssClient>(
				config.ws_reverse_event_url.substr(strlen("wss://")));
			ws_reverse_event_client_is_wss_ = true;
		}
	} catch (...) {
		// in case "init_ws_reverse_client()" failed due to invalid "server_port_path"
		ws_reverse_event_client_is_wss_ = nullopt;
	}
}

void ApiServer::finalize_ws_reverse() {
	ws_reverse_api_client_.ws = nullptr;
	ws_reverse_api_client_.wss = nullptr;
	ws_reverse_api_client_is_wss_ = nullopt;
	ws_reverse_api_client_started_ = false;

	ws_reverse_event_client_.ws = nullptr;
	ws_reverse_event_client_.wss = nullptr;
	ws_reverse_event_client_is_wss_ = nullopt;
	ws_reverse_event_client_started_ = false;
}

void ApiServer::start_ws_reverse() {
	if (config.use_ws_reverse) {
		init_ws_reverse();

		if (ws_reverse_api_client_is_wss_.has_value()) {
			// client successfully initialized
			ws_reverse_api_thread_ = thread([&]() {
				ws_reverse_api_client_started_ = true;
				try {
					if (ws_reverse_api_client_is_wss_.value() == false) {
						ws_reverse_api_client_.ws->start();
					} else {
						ws_reverse_api_client_.wss->start();
					}
				} catch (...) {}
				ws_reverse_api_client_started_ = false;
			});
			Log::d(TAG, u8"开启 API WebSocket 反向客户端成功，开始连接 " + config.ws_reverse_api_url);
		}

		if (ws_reverse_event_client_is_wss_.has_value()) {
			// client successfully initialized
			ws_reverse_event_thread_ = thread([&]() {
				ws_reverse_event_client_started_ = true;
				try {
					if (ws_reverse_event_client_is_wss_.value() == false) {
						ws_reverse_event_client_.ws->start();
					} else {
						ws_reverse_event_client_.wss->start();
					}
				} catch (...) {}
				ws_reverse_event_client_started_ = false;
			});
			Log::d(TAG, u8"开启 Event WebSocket 反向客户端成功，开始连接 " + config.ws_reverse_event_url);
		}
	}
}

void ApiServer::stop_ws_reverse() {
	if (ws_reverse_api_client_started_) {
		if (ws_reverse_api_client_is_wss_.value() == false) {
			// the WsClient class is modified by us ("connection" property made public),
			// so we must maintain the lock manually
			unique_lock<mutex> lock(ws_reverse_api_client_.ws->connection_mutex);
			ws_reverse_api_client_.ws->connection->send_close(1000);
			lock.unlock();
			ws_reverse_api_client_.ws->stop();
		} else {
			unique_lock<mutex> lock(ws_reverse_api_client_.wss->connection_mutex);
			ws_reverse_api_client_.wss->connection->send_close(1000);
			lock.unlock();
			ws_reverse_api_client_.wss->stop();
		}
		ws_reverse_api_client_started_ = false;
	}
	if (ws_reverse_api_thread_.joinable()) {
		ws_reverse_api_thread_.join();
	}

	if (ws_reverse_event_client_started_) {
		if (ws_reverse_event_client_is_wss_.value() == false) {
			unique_lock<mutex> lock(ws_reverse_event_client_.ws->connection_mutex);
			ws_reverse_event_client_.ws->connection->send_close(1000);
			lock.unlock();
			ws_reverse_event_client_.ws->stop();
		} else {
			unique_lock<mutex> lock(ws_reverse_event_client_.wss->connection_mutex);
			ws_reverse_event_client_.wss->connection->send_close(1000);
			lock.unlock();
			ws_reverse_event_client_.wss->stop();
		}
		ws_reverse_event_client_started_ = false;
	}
	if (ws_reverse_event_thread_.joinable()) {
		ws_reverse_event_thread_.join();
	}

	finalize_ws_reverse();
}
