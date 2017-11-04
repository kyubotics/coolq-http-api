// 
// server_class.h : Define ApiServer class.
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

#pragma once

#include "common.h"

#include "web_server/server_http.hpp"
#include "web_server/server_ws.hpp"
#include "web_server/client_ws.hpp"
#include "web_server/client_wss.hpp"

class ApiServer {
public:
    static ApiServer &instance() {
        static ApiServer instance;
        return instance;
    }

    void start();
    void stop();

    bool is_initialized() const { return initialized_; }
    bool http_server_is_started() const { return http_server_started_; }
    bool ws_server_is_started() const { return ws_server_started_; }
    bool ws_reverse_api_client_is_started() const { return ws_reverse_api_client_started_; }

    // websocket only
    void push_event(const json &payload) const;

private:
    ApiServer() {}

    ~ApiServer() {
        stop();
    }

    ApiServer(const ApiServer &) = delete;
    void operator=(const ApiServer &) = delete;

    bool initialized_ = false;

    // http api server
    std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>> http_server_;
    std::thread http_thread_;
    bool http_server_started_ = false;

    // websocket api and event server
    std::shared_ptr<SimpleWeb::SocketServer<SimpleWeb::WS>> ws_server_;
    std::thread ws_thread_;
    bool ws_server_started_ = false;

    // reverse websocket
    union WsReverseApiClient {
        std::shared_ptr<SimpleWeb::SocketClient<SimpleWeb::WS>> ws;
        std::shared_ptr<SimpleWeb::SocketClient<SimpleWeb::WSS>> wss;

        WsReverseApiClient() : ws(nullptr) {}
        ~WsReverseApiClient() {}
    };

    // reverse websocket api client
    WsReverseApiClient ws_reverse_api_client_;
    std::optional<bool> ws_reverse_api_client_is_wss_;
    std::thread ws_reverse_api_thread_;
    bool ws_reverse_api_client_started_ = false;

    // reverse websocket event client
    WsReverseApiClient ws_reverse_event_client_;
    std::optional<bool> ws_reverse_event_client_is_wss_;
    std::thread ws_reverse_event_thread_;
    bool ws_reverse_event_client_started_ = false;

    /**
     * This will be called in start().
     */
    void init();

    /**
     * Init http api server.
     */
    void init_http();

    /**
     * Init websocket server.
     */
    void init_ws();

    /**
     * Init reverse websocket client.
     */
    void init_ws_reverse();

    /**
    * This will be called in stop().
    */
    void finalize();
    void finalize_http();
    void finalize_ws();
    void finalize_ws_reverse();
};
