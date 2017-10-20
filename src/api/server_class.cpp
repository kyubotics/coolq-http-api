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

#include "./server_class.h"

#include "app.h"

#include <boost/filesystem.hpp>

#include "types.h"
#include "utils/params_class.h"

using namespace std;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

extern ApiHandlerMap api_handlers; // defined in handlers.cpp

static const auto TAG = u8"API服务";

static bool authorize(const decltype(HttpServer::Request::header) &headers, const json &query_args,
                      const function<void(SimpleWeb::StatusCode)> on_failed = nullptr) {
    if (config.access_token.empty()) {
        return true;
    }

    string token_given;
    if (const auto headers_it = headers.find("Authorization");
        headers_it != headers.end()
        && (boost::starts_with(headers_it->second, "Token ") || boost::starts_with(headers_it->second, "token "))) {
        token_given = headers_it->second.substr(strlen("Token "));
    } else if (const auto args_it = query_args.find("access_token"); args_it != query_args.end()) {
        token_given = args_it->get<string>();
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

void ApiServer::init() {
    Log::d(TAG, u8"初始化 API 处理函数");
    init_http();
    init_ws();
    initialized_ = true;
}

void ApiServer::init_http() {
    http_server_.default_resource["GET"]
            = http_server_.default_resource["POST"]
            = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
                response->write(SimpleWeb::StatusCode::client_error_not_found);
            };

    for (const auto &handler_kv : api_handlers) {
        const auto path_regex = "^/" + handler_kv.first + "$";
        http_server_.resource[path_regex]["GET"] = http_server_.resource[path_regex]["POST"]
                = [&handler_kv](shared_ptr<HttpServer::Response> response,
                                shared_ptr<HttpServer::Request> request) {
                    Log::d(TAG, u8"收到 API 请求：" + request->method
                           + u8" " + request->path
                           + (request->query_string.empty() ? "" : "?" + request->query_string));

                    auto json_params = json::object();
                    json args = request->parse_query_string(), form;

                    auto authorized = authorize(request->header, args, [&response](auto status_code) {
                        response->write(status_code);
                    });
                    if (!authorized) {
                        Log::d(TAG, u8"没有提供 Token 或 Token 不符，已拒绝请求");
                        return;
                    }

                    if (request->method == "POST") {
                        string content_type;
                        if (const auto it = request->header.find("Content-Type");
                            it != request->header.end()) {
                            content_type = it->second;
                            Log::d(TAG, u8"Content-Type: " + content_type);
                        }

                        if (boost::starts_with(content_type, "application/x-www-form-urlencoded")) {
                            form = SimpleWeb::QueryString::parse(request->content.string());
                        } else if (boost::starts_with(content_type, "application/json")) {
                            try {
                                json_params = json::parse(request->content.string()); // may throw invalid_argument
                                if (!json_params.is_object()) {
                                    throw invalid_argument("must be a JSON object");
                                }
                            } catch (invalid_argument &) {
                                Log::d(TAG, u8"HTTP 正文的 JSON 无效或者不是对象");
                                response->write(SimpleWeb::StatusCode::client_error_bad_request);
                                return;
                            }
                        } else {
                            Log::d(TAG, u8"Content-Type 不支持");
                            response->write(SimpleWeb::StatusCode::client_error_not_acceptable);
                            return;
                        }
                    }

                    // merge form and args to json params
                    for (auto data : {form, args}) {
                        if (data.is_object()) {
                            for (auto it = data.begin(); it != data.end(); ++it) {
                                json_params[it.key()] = it.value();
                            }
                        }
                    }

                    Log::d(TAG, u8"API 处理函数 " + handler_kv.first + u8" 开始处理请求");
                    ApiResult result;
                    Params params(json_params);
                    handler_kv.second(params, result);

                    decltype(request->header) headers{
                        {"Content-Type", "application/json; charset=UTF-8"}
                    };
                    auto resp_body = result.json().dump();
                    Log::d(TAG, u8"响应数据已准备完毕：" + resp_body);
                    response->write(resp_body, headers);
                    Log::d(TAG, u8"响应内容已发送");
                    Log::i(TAG, u8"已成功处理一个 API 请求：" + request->path);
                };
    }

    const auto regex = "^/(data/(?:bface|image|record|show)/.+)$";
    http_server_.resource[regex]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                             shared_ptr<HttpServer::Request> request) {
        if (!config.serve_data_files) {
            response->write(SimpleWeb::StatusCode::client_error_not_found);
            return;
        }

        auto relpath = request->path_match.str(1);
        boost::algorithm::replace_all(relpath, "/", "\\");
        Log::d(TAG, u8"收到 GET 数据文件请求，相对路径：" + relpath);

        if (boost::algorithm::contains(relpath, "..")) {
            Log::d(TAG, u8"请求的数据文件路径中有非法字符，已拒绝请求");
            response->write(SimpleWeb::StatusCode::client_error_forbidden);
            return;
        }

        auto filepath = sdk->directories().coolq() + relpath;
        auto ansi_filepath = ansi(filepath);
        if (!boost::filesystem::is_regular_file(ansi_filepath)) {
            // is not a file
            Log::d(TAG, u8"相对路径 " + relpath + u8" 所制定的内容不存在，或为非文件类型，无法发送");
            response->write(SimpleWeb::StatusCode::client_error_not_found);
            return;
        }

        if (ifstream f(ansi_filepath, ios::in | ios::binary); f.is_open()) {
            auto length = boost::filesystem::file_size(ansi_filepath);
            response->write(decltype(request->header){
                {"Content-Length", to_string(length)},
                {"Content-Type", "application/octet-stream"},
                {"Content-Disposition", "attachment"}
            });
            *response << f.rdbuf();
            Log::d(TAG, u8"文件内容已发送完毕");
            f.close();
        } else {
            Log::d(TAG, u8"文件 " + relpath + u8" 打开失败，请检查文件系统权限");
            response->write(SimpleWeb::StatusCode::client_error_forbidden);
            return;
        }

        Log::i(TAG, u8"已成功发送文件：" + relpath);
    };
}

void ApiServer::init_ws() {
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

    auto &api_endpoint = ws_server_.endpoint["^/api/?$"];
    api_endpoint.on_open = on_open_callback;
    api_endpoint.on_message = [](shared_ptr<WsServer::Connection> connection,
                                 shared_ptr<WsServer::Message> message) {
        auto ws_message_str = message->string();
        Log::d(TAG, u8"收到 API 请求（WebSocket）：" + ws_message_str);

        ApiResult result;

        auto send_result = [&]() {
            auto resp_body = result.json().dump();
            Log::d(TAG, u8"响应数据已准备完毕：" + resp_body);
            auto send_stream = make_shared<WsServer::SendStream>();
            *send_stream << resp_body;
            connection->send(send_stream);
            Log::d(TAG, u8"响应内容已发送");
        };

        json payload;
        try {
            payload = json::parse(ws_message_str);
        } catch (invalid_argument &) {
            // bad JSON
        }
        if (!(payload.is_object() && payload.find("action") != payload.end() && payload["action"].is_string())) {
            Log::d(TAG, u8"消息中的 JSON 无效或者不是对象");
            result.retcode = ApiResult::RetCodes::HTTP_BAD_REQUEST;
            send_result();
            return;
        }

        auto action = payload["action"].get<string>();
        ApiHandler handler;
        if (auto it = api_handlers.find(action); it != api_handlers.end()) {
            Log::d(TAG, u8"找到 API 处理函数 " + action + u8"，开始处理请求");
            handler = it->second;
        } else {
            Log::d(TAG, u8"未找到 API 处理函数 " + action);
            result.retcode = ApiResult::RetCodes::HTTP_NOT_FOUND;
            send_result();
            return;
        }

        auto json_params = json::object();
        if (payload.find("params") != payload.end() && payload["params"].is_object()) {
            json_params = payload["params"];
        }

        Params params(json_params);
        handler(params, result);

        send_result();
    };

    auto &event_endpoint = ws_server_.endpoint["^/event/?$"];
    event_endpoint.on_open = on_open_callback;
}

void ApiServer::start() {
    if (!initialized_) {
        return;
    }

    if (config.use_http) {
        http_server_.config.address = config.host;
        http_server_.config.port = config.port;
        http_thread_ = thread([&]() {
            http_server_.start();
            http_server_started_ = false; // since it reaches here, the server is absolutely stopped
        });
        http_server_started_ = true;
        Log::d(TAG, u8"开启 API HTTP 服务器成功，开始监听 http://"
               + http_server_.config.address + ":" + to_string(http_server_.config.port));
    }

    if (config.use_ws) {
        ws_server_.config.address = config.ws_host;
        ws_server_.config.port = config.ws_port;
        ws_thread_ = thread([&]() {
            ws_server_.start();
            ws_server_started_ = false;
        });
        ws_server_started_ = true;
        Log::d(TAG, u8"开启 API WebSocket 服务器成功，开始监听 http://"
               + ws_server_.config.address + ":" + to_string(ws_server_.config.port));
    }
}

void ApiServer::stop() {
    if (http_server_started_) {
        http_server_.stop();
        if (http_thread_.joinable()) {
            http_thread_.join();
        }
        http_server_started_ = false;
    }
    if (ws_server_started_) {
        ws_server_.stop();
        if (ws_thread_.joinable()) {
            ws_thread_.join();
        }
        ws_server_started_ = false;
    }
    Log::d(TAG, u8"已关闭 API 服务");
}

size_t ApiServer::push_event(const json &payload) {
    if (!ws_server_started_) {
        return 0;
    }

    size_t count = 0;
    for (const auto &connection : ws_server_.get_connections()) {
        if (boost::algorithm::starts_with(connection->path, "/event")) {
            const auto send_stream = make_shared<WsServer::SendStream>();
            *send_stream << payload.dump();
            connection->send(send_stream);
            count++;
        }
    }
    return count;
}
