// 
// main_handler.cpp : Implement main generic handler.
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

#include "main_handler.h"

#include "app.h"

#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>

#include "Message.h"
#include "types.h"

using namespace std;

extern ApiHandlerMap api_handlers; // global handler map in handlers.cpp

void static_file_handler(evhttp_request *req, str path); // implemented in static_file_handler.cpp
static ApiResult dispatch_request(const ApiRequest &request);

static ApiRequest new_request() {
    ApiRequest request;
    auto evkeyvalq_size = sizeof(evkeyvalq);
    request.args = new evkeyvalq;
    memset(request.args, 0, evkeyvalq_size);
    request.form = new evkeyvalq;
    memset(request.form, 0, evkeyvalq_size);
    request.json = nullptr;
    return request;
}

static void cleanup_request(ApiRequest &request) {
    // clear args, form, and json
    if (request.args) {
        evhttp_clear_headers(request.args);
        delete request.args;
        request.args = nullptr;
    }
    if (request.form) {
        evhttp_clear_headers(request.form);
        delete request.form;
        request.form = nullptr;
    }
    if (request.json) {
        json_decref(request.json);
        request.json = nullptr;
    }
}

void api_main_handler(evhttp_request *req, void *_) {
    auto method = evhttp_request_get_command(req);
    if (!(method & (EVHTTP_REQ_GET | EVHTTP_REQ_POST))) {
        // method not supported
        evhttp_send_error(req, HTTP_BADMETHOD, nullptr);
        return;
    }

    L.d("API请求", str("收到 API 请求，请求方式：") + (method == EVHTTP_REQ_GET ? "GET" : "POST") + "，URI：" + evhttp_request_get_uri(req));

    auto input_headers = evhttp_request_get_input_headers(req);

    auto expected_token = CQ->config.token;
    str token = "";

    // try to get token from headers
    auto auth = evhttp_find_header(input_headers, "Authorization");
    if (auth) {
        token = auth;
        if (token.startswith("token ")) {
            // it's a valid token
            token = token[slice(strlen("token "))].strip();
        } else {
            token = "";
        }
    }

    //    if (expected_token) {
    //        auto auth = evhttp_find_header(input_headers, "Authorization");
    //        if (!auth || "token " + expected_token != auth) {
    //            // invalid token
    //            L.d("API请求", "token 不符，停止响应");
    //            L.i("API请求", "有未经授权的 API 请求，已拒绝");
    //            evhttp_send_error(req, 401, nullptr);
    //            return;
    //        }
    //    }

    auto uri = evhttp_request_get_evhttp_uri(req);

    auto path = str(evhttp_uri_get_path(uri));
    if (path.startswith("/data/") && CQ->config.serve_data_file) {
        // let static_file_handler take care of this
        static_file_handler(req, path);
        return;
    }

    // initialize request
    auto request = new_request();
    request.path = path;

    // parse query arguments
    auto encoded_args_c_str = evhttp_uri_get_query(uri);
    evhttp_parse_query_str(encoded_args_c_str ? encoded_args_c_str : "", request.args);

    // try to get token from query params
    if (!token) {
        token = request.get_str_param("access_token", "");
    }
    if (expected_token && token != expected_token) {
        // we expect a token, but the given token does not match
        L.d("API请求", "token 不符，停止响应");
        L.i("API请求", "有未经授权的 API 请求，已拒绝");
        evhttp_send_error(req, 401, nullptr);
        cleanup_request(request);
        return;
    }

    if (method == EVHTTP_REQ_POST) {
        // check content type
        auto content_type_c_str = evhttp_find_header(input_headers, "Content-Type");
        auto content_type = string(content_type_c_str ? content_type_c_str : "");
        if (content_type == "application/x-www-form-urlencoded" || content_type == "application/json") {
            // read request body as string
            auto input_buffer = evhttp_request_get_input_buffer(req);
            auto length = evbuffer_get_length(input_buffer);
            char *request_body;
            if (length > 0) {
                request_body = new char[length + 1];
                memcpy(request_body, evbuffer_pullup(input_buffer, -1), length);
                request_body[length] = '\0';
            } else {
                request_body = new char[1];
                request_body[0] = '\0'; // an empty string on heap
            }

            if (content_type == "application/x-www-form-urlencoded") {
                // parse form
                evhttp_parse_query_str(request_body, request.form);
            } else {
                // parse json
                request.json = json_loads(request_body, 0, nullptr);
            }

            delete[] request_body;
        } else {
            // no content-type or unrecognized content-type
            L.d("API请求", "API 请求未指定或指定了不支持的 Content-Type");
            evhttp_send_error(req, HTTP_BADREQUEST, nullptr);
            cleanup_request(request);
            return;
        }
    }

    // dispatch
    auto result = dispatch_request(request);
    cleanup_request(request);

    if (result.retcode == ApiRetCode::NO_SUCH_API) {
        // no such API, should return 404 Not Found
        evhttp_send_error(req, HTTP_NOTFOUND, nullptr);
        return;
    }

    // set headers
    auto output_headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(output_headers, "Content-Type", "application/json; charset=UTF-8");
    evhttp_add_header(output_headers, "Connection", "close");

    // write response
    auto buf = evbuffer_new();
    auto json = json_pack("{s:s,s:i,s:o?}",
                          "status", result.retcode == ApiRetCode::OK ? "ok" : "failed",
                          "retcode", result.retcode,
                          "data", result.data);
    auto json_str = json_dumps(json, JSON_COMPACT);

    evbuffer_add_printf(buf, "%s", json_str);
    evhttp_send_reply(req, HTTP_OK, nullptr, buf);
    L.d("API请求", "响应内容发送完毕");

    free(json_str); // free the memory that "json_dumps" allocated
    json_decref(json);
    evbuffer_free(buf);

    L.i("API请求", "已成功处理一个 API 请求：" + request.path);
}

static ApiResult dispatch_request(const ApiRequest &request) {
    auto path_without_slash = request.path[slice(1)];
    ApiResult result;
    if (api_handlers.find(path_without_slash.c_str()) != api_handlers.end()) {
        // the corresponding handler exists
        L.d("API请求", "找到处理函数：" + path_without_slash);
        api_handlers[path_without_slash.c_str()](request, result);
        return result;
    }
    // the corresponding handler does not exist
    L.d("API请求", "没有找到处理函数：" + path_without_slash);
    result.retcode = ApiRetCode::NO_SUCH_API;
    return result;
}
