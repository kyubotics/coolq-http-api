#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    /**
     * Common "on_message" callback for websocket server's api endpoint and reverse websocket api client.
     * \tparam WsT WsServer (websocket server /api/ endpoint) or WsClient (reverse websocket api client)
     */
    template <typename WsT>
    static void ws_api_on_message(std::shared_ptr<typename WsT::Connection> connection,
                                  std::shared_ptr<typename WsT::Message> message) {
        static const auto TAG = u8"WS通用";

        const auto ws_message_str = message->string();
        logging::debug(TAG, u8"收到 API 请求：" + ws_message_str);

        const auto send_result = [&connection](const ActionResult &result, const json &echo = nullptr) {
            json resp_json = result;
            if (!echo.is_null()) {
                resp_json["echo"] = echo;
            }
            const auto resp_body = resp_json.dump();
            logging::debug(TAG, u8"响应数据已准备完毕：" + resp_body);
            const auto send_stream = std::make_shared<typename WsT::SendStream>();
            *send_stream << resp_body;
            connection->send(send_stream);
            logging::debug(TAG, u8"响应内容已发送");
        };

        json payload;
        try {
            payload = json::parse(ws_message_str);
        } catch (json::parse_error &) {
            // bad JSON
        }
        if (!(payload.is_object() && payload.find("action") != payload.end() && payload["action"].is_string())) {
            logging::debug(TAG, u8"请求中的 JSON 无效或者不是对象");
            send_result(ActionResult(ActionResult::Codes::HTTP_BAD_REQUEST));
            return;
        }

        const auto action = payload["action"].get<std::string>();

        auto params = json::object();
        if (payload.find("params") != payload.end() && payload["params"].is_object()) {
            params = payload["params"];
        }

        logging::debug(TAG, u8"开始执行动作 " + action);
        const auto result = call_action(action, params);
        if (result.code != ActionResult::Codes::HTTP_NOT_FOUND) {
            logging::debug(TAG, u8"动作 " + action + u8" 执行成功");
        } else {
            logging::debug(TAG, u8"没有找到相应的处理函数，动作 " + action + u8" 执行失败");
        }

        json echo;
        try {
            echo = payload.at("echo");
        } catch (...) {
        }

        send_result(result, echo);
    }
} // namespace cqhttp::plugins
