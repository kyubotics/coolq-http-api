#pragma once

#include "app.h"

#include <boost/filesystem.hpp>

#include "api/api.h"
#include "web_server/utility.hpp"

namespace fs = boost::filesystem;

static const auto TAG = u8"API服务";

/**
 * Do authorization (check access token),
 * should be called on incomming connection request (http server and websocket server)
 */
static bool authorize(const SimpleWeb::CaseInsensitiveMultimap &headers, const json &query_args,
                      const std::function<void(SimpleWeb::StatusCode)> on_failed = nullptr) {
    if (config.access_token.empty()) {
        return true;
    }

    std::string token_given;
    if (const auto headers_it = headers.find("Authorization");
        headers_it != headers.end()
        && (boost::starts_with(headers_it->second, "Token ") || boost::starts_with(headers_it->second, "token "))) {
        token_given = headers_it->second.substr(strlen("Token "));
    } else if (const auto args_it = query_args.find("access_token"); args_it != query_args.end()) {
        token_given = args_it->get<std::string>();
    }

    if (token_given.empty()) {
        if (on_failed) {
            on_failed(SimpleWeb::StatusCode::client_error_unauthorized);
        }
        return false;
    }

    if (token_given != config.access_token) {
        if (on_failed) {
            on_failed(SimpleWeb::StatusCode::client_error_forbidden);
        }
        return false;
    }

    return true; // token_given == config.token
}

static auto server_thread_pool_size() {
    return config.server_thread_pool_size > 0
               ? config.server_thread_pool_size
               : std::thread::hardware_concurrency() * 2 + 1;
}

/**
 * \brief Common "on_message" callback for websocket server's api endpoint and reverse websocket api client.
 * \tparam WsT WsServer (websocket server /api/ endpoint) or WsClient (reverse websocket api client)
 */
template <typename WsT>
static void ws_api_on_message(std::shared_ptr<typename WsT::Connection> connection,
                              std::shared_ptr<typename WsT::Message> message) {
    auto ws_message_str = message->string();
    Log::d(TAG, u8"收到 API 请求（WebSocket）：" + ws_message_str);

    ApiResult result;

    auto send_result = [&]() {
        auto resp_body = result.json().dump();
        Log::d(TAG, u8"响应数据已准备完毕：" + resp_body);
        auto send_stream = std::make_shared<typename WsT::SendStream>();
        *send_stream << resp_body;
        connection->send(send_stream);
        Log::d(TAG, u8"响应内容已发送");
    };

    json payload;
    try {
        payload = json::parse(ws_message_str);
    } catch (std::invalid_argument &) {
        // bad JSON
    }
    if (!(payload.is_object() && payload.find("action") != payload.end() && payload["action"].is_string())) {
        Log::d(TAG, u8"消息中的 JSON 无效或者不是对象");
        result.retcode = ApiResult::RetCodes::HTTP_BAD_REQUEST;
        send_result();
        return;
    }

    const auto action = payload["action"].get<std::string>();

    auto json_params = json::object();
    if (payload.find("params") != payload.end() && payload["params"].is_object()) {
        json_params = payload["params"];
    }
    const Params params(json_params);

    try {
        invoke_api(action, params, result);
        Log::d(TAG, u8"找到 API 处理函数 " + action + u8"，已成功处理请求");
    } catch (std::invalid_argument &) {
        Log::d(TAG, u8"未找到 API 处理函数 " + action);
        result.retcode = ApiResult::RetCodes::HTTP_NOT_FOUND;
    }

    send_result();
}
