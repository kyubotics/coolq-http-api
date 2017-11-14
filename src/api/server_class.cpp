// 
// server_class.cpp : Implement ApiServer class.
// 
// Copyright (C) 2017  Richard Chien <richardchienthebest@gmail.com>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

#include "./server_impl_common.h"

using namespace std;
using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;
using WssClient = SimpleWeb::SocketClient<SimpleWeb::WSS>;

void ApiServer::start() {
	start_http();
	start_ws();
	start_ws_reverse();

	Log::d(TAG, u8"已开启 API 服务");
}

void ApiServer::stop() {
	stop_http();
	stop_ws();
	stop_ws_reverse();

	Log::d(TAG, u8"已关闭 API 服务");
}

void ApiServer::push_event(const json &payload) const {
	if (ws_server_started_) {
		Log::d(TAG, u8"开始通过 WebSocket 服务端推送事件");
		size_t total_count = 0;
		size_t succeeded_count = 0;
		for (const auto &connection : ws_server_->get_connections()) {
			if (boost::algorithm::starts_with(connection->path, "/event")) {
				total_count++;
				try {
					const auto send_stream = make_shared<WsServer::SendStream>();
					*send_stream << payload.dump();
					connection->send(send_stream);
					succeeded_count++;
				} catch (...) {}
			}
		}
		Log::d(TAG, u8"已成功向 " + to_string(succeeded_count) + "/" + to_string(total_count) + u8" 个 WebSocket 客户端推送事件");
	}

	if (ws_reverse_event_client_started_) {
		Log::d(TAG, u8"开始通过 WebSocket 反向客户端上报事件");

		bool succeeded;
		try {
			if (ws_reverse_event_client_is_wss_.value() == false) {
				const auto send_stream = make_shared<WsClient::SendStream>();
				*send_stream << payload.dump();
				ws_reverse_event_client_.ws->connection->send(send_stream);
			} else {
				const auto send_stream = make_shared<WssClient::SendStream>();
				*send_stream << payload.dump();
				ws_reverse_event_client_.wss->connection->send(send_stream);
			}
			succeeded = true;
		} catch (...) {
			succeeded = false;
		}

		Log::d(TAG, u8"通过 WebSocket 反向客户端上报数据到 " + config.ws_reverse_event_url + (succeeded ? u8" 成功" : u8" 失败"));
	}
}
