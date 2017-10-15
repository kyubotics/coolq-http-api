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

#include "types.h"

using namespace std;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using Response = HttpServer::Response;
using Request = HttpServer::Request;

extern ApiHandlerMap api_handlers; // defined in handlers.cpp

class ResponseHelper {
public:
    unsigned short status_code;
    bytes body;
    SimpleWeb::CaseInsensitiveMultimap headers;

    void write(const shared_ptr<Response> response) {
        *response << "HTTP/1.1 " << SimpleWeb::status_code(SimpleWeb::StatusCode(status_code)) << "\r\n";
        headers.erase("Content-Length");
        headers.emplace("Content-Length", to_string(body.size()));
        for (const auto &kv : headers) {
            *response << kv.first << ": " << kv.second << "\r\n";
        }
        *response << "\r\n" << body;
    }
};

static const auto TAG = u8"API服务";

static bool authorize(const decltype(Request::header) &headers, const json &query_args,
                      const function<void(unsigned short)> on_failed = nullptr) {
    if (config.access_token.empty()) {
        return true;
    }

    string token_given;
    if (const auto headers_it = headers.find("Authorization");
        headers_it != headers.end() && boost::starts_with(headers_it->second, "Token ")) {
        token_given = headers_it->second.substr(strlen("Token "));
    } else if (const auto args_it = query_args.find("access_token"); args_it != query_args.end()) {
        token_given = args_it->get<string>();
    }

    if (token_given.empty()) {
        if (on_failed) {
            on_failed(401 /* Unauthorized */);
        }
        return false;
    }

    if (token_given != config.access_token) {
        if (on_failed) {
            on_failed(403 /* Forbidden */);
        }
        return false;
    }

    return true; // token_given == config.token
}

void ApiServer::init() {
    Log::d(TAG, u8"初始化 API 处理函数");

    server_.default_resource["GET"]
            = server_.default_resource["POST"]
            = [](shared_ptr<Response> response, shared_ptr<Request> request) {
                ResponseHelper{404 /* Not Found */}.write(response);
            };

    for (const auto &handler_kv : api_handlers) {
        const auto path_regex = "^/" + handler_kv.first + "$";
        server_.resource[path_regex]["GET"] = server_.resource[path_regex]["POST"]
                = [&handler_kv](shared_ptr<Response> response, shared_ptr<Request> request) {
                    Log::d(TAG, u8"收到 API 请求：" + request->method
                           + u8" " + request->path + u8"?" + request->query_string);

                    auto json_params = json::object();
                    json args = request->parse_query_string(), form;

                    auto authorized = authorize(request->header, args, [&response](auto status_code) {
                        ResponseHelper{status_code}.write(response);
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

                        if (content_type == "application/x-www-form-urlencoded") {
                            form = SimpleWeb::QueryString::parse(request->content.string());
                        } else if (content_type == "application/json") {
                            try {
                                json_params = json::parse(request->content.string()); // may throw invalid_argument
                                if (!json_params.is_object()) {
                                    throw invalid_argument("must be a JSON object");
                                }
                            } catch (invalid_argument &) {
                                Log::d(TAG, u8"HTTP 正文的 JSON 无效或者不是对象");
                                ResponseHelper{400 /* Bad Request */}.write(response);
                                return;
                            }
                        } else {
                            Log::d(TAG, u8"Content-Type 不支持");
                            ResponseHelper{406 /* Not Acceptable */}.write(response);
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
                    ApiParams params(json_params);
                    handler_kv.second(params, result);

                    decltype(request->header) headers{
                        {"Server", CQAPP_SERVER},
                        {"Content-Type", "application/json; charset=UTF-8"}
                    };
                    auto resp_body = result.json().dump();
                    Log::d(TAG, u8"响应数据已准备完毕：" + resp_body);
                    response->write(resp_body, headers);
                    Log::d(TAG, u8"响应内容已发送");
                    Log::i(TAG, u8"已成功处理一个 API 请求：" + request->path);
                };
    }

    initiated_ = true;
}

void ApiServer::start(const string &host, const unsigned short port) {
    server_.config.address = host;
    server_.config.port = port;
    thread_ = std::thread([&]() {
        server_.start();
    });
    Log::d(TAG, u8"开启 API 服务成功，开始监听 http://" + host + ":" + to_string(port));
}

void ApiServer::stop() {
    server_.stop();
    if (thread_.joinable()) {
        thread_.join();
    }
    Log::d(TAG, u8"已关闭 API 服务");
}
