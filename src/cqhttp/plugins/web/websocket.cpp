#include "./websocket.h"

#include "cqhttp/plugins/web/server_common.h"
#include "cqhttp/plugins/web/ws_common.h"

using namespace std;
using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

namespace cqhttp::plugins {
    static const auto TAG = "WS";

    void WebSocket::init_server() {
        logging::debug(TAG, u8"初始化 WebSocket");

        auto on_open_callback = [=](shared_ptr<WsServer::Connection> connection) {
            logging::debug(TAG, u8"收到 WebSocket 连接：" + connection->path);
            const json args = SimpleWeb::QueryString::parse(connection->query_string);
            const auto authorized = authorize(access_token_, connection->header, args);
            if (!authorized) {
                logging::debug(TAG, u8"没有提供 Token 或 Token 不符，已关闭连接");
                const auto send_stream = make_shared<WsServer::SendStream>();
                *send_stream << "authorization failed";
                connection->send(send_stream);
                connection->send_close(1000); // we don't want this client any more
            }
        };

        server_ = make_shared<WsServer>();

        auto &api_endpoint = server_->endpoint["^/api/?$"];
        api_endpoint.on_open = on_open_callback;
        api_endpoint.on_message = ws_api_on_message<WsServer>;

        auto &event_endpoint = server_->endpoint["^/event/?$"];
        event_endpoint.on_open = on_open_callback;
    }

    void WebSocket::hook_enable(Context &ctx) {
        use_ws_ = ctx.config->get_bool("use_ws", false);
        access_token_ = ctx.config->get_string("access_token", "");

        if (use_ws_) {
            init_server();

            server_->config.thread_pool_size =
                fix_server_thread_pool_size(ctx.config->get_integer("server_thread_pool_size", 1));
            server_->config.address = ctx.config->get_string("ws_host", "0.0.0.0");
            server_->config.port = ctx.config->get_integer("ws_port", 6700);
            thread_ = thread([&]() {
                started_ = true;
                try {
                    server_->start(); // loop to accept requests
                } catch (...) {
                }
                started_ = false; // since it reaches here, the server is absolutely stopped
            });
            logging::info_success(TAG,
                                  u8"开启 API WebSocket 服务器成功，开始监听 ws://" + server_->config.address + ":"
                                      + to_string(server_->config.port));
        }

        ctx.next();
    }

    void WebSocket::hook_disable(Context &ctx) {
        if (started_) {
            server_->stop();
            started_ = false;
        }
        if (thread_.joinable()) {
            thread_.join();
        }

        server_ = nullptr;

        ctx.next();
    }

    void WebSocket::hook_after_event(EventContext<cq::Event> &ctx) {
        if (started_) {
            logging::debug(TAG, u8"开始通过 WebSocket 服务端推送事件");
            size_t total_count = 0;
            size_t succeeded_count = 0;
            for (const auto &connection : server_->get_connections()) {
                if (boost::algorithm::starts_with(connection->path, "/event")) {
                    total_count++;
                    try {
                        const auto send_stream = make_shared<WsServer::SendStream>();
                        *send_stream << ctx.data.dump();
                        connection->send(send_stream);
                        succeeded_count++;
                    } catch (...) {
                    }
                }
            }
            logging::info_success(TAG,
                                  u8"已成功向 " + to_string(succeeded_count) + "/" + to_string(total_count)
                                      + u8" 个 WebSocket 客户端推送事件");
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
