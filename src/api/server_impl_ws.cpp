#include "./server_impl_common.h"

using namespace std;
using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

void ApiServer::init_ws() {
	Log::d(TAG, u8"初始化 WebSocket");

	auto on_open_callback = [](shared_ptr<WsServer::Connection> connection) {
		Log::d(TAG, u8"收到 WebSocket 连接：" + connection->path);
		json args = SimpleWeb::QueryString::parse(connection->query_string);
		auto authorized = authorize(connection->header, args);
		if (!authorized) {
			Log::d(TAG, u8"没有提供 Token 或 Token 不符，已关闭连接");
			auto send_stream = make_shared<WsServer::SendStream>();
			*send_stream << "authorization failed";
			connection->send(send_stream);
			connection->send_close(1000); // we don't want this client any more
			return;
		}
	};

	ws_server_ = make_shared<WsServer>();

	auto &api_endpoint = ws_server_->endpoint["^/api/?$"];
	api_endpoint.on_open = on_open_callback;
	api_endpoint.on_message = ws_api_on_message<WsServer>;

	auto &event_endpoint = ws_server_->endpoint["^/event/?$"];
	event_endpoint.on_open = on_open_callback;
}

void ApiServer::finalize_ws() {
	ws_server_ = nullptr;
	ws_server_started_ = false;
}

void ApiServer::start_ws() {
	if (config.use_ws) {
		init_ws();

		ws_server_->config.thread_pool_size = server_thread_pool_size();
		ws_server_->config.address = config.ws_host;
		ws_server_->config.port = config.ws_port;
		ws_thread_ = thread([&]() {
			ws_server_started_ = true;
			try {
				ws_server_->start();
			} catch (...) {}
			ws_server_started_ = false;
		});
		Log::d(TAG, u8"开启 API WebSocket 服务器成功，开始监听 ws://"
		       + ws_server_->config.address + ":" + to_string(ws_server_->config.port));
	}
}

void ApiServer::stop_ws() {
	if (ws_server_started_) {
		ws_server_->stop();
		ws_server_started_ = false;
	}
	if (ws_thread_.joinable()) {
		ws_thread_.join();
	}
	finalize_ws();
}
