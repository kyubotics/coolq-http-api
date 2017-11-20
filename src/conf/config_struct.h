// 
// config_struct.h : Define the Config structure.
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

struct Config {
    std::string host = "0.0.0.0";
    unsigned short port = 5700;
    bool use_http = true;
    std::string ws_host = "0.0.0.0";
    unsigned short ws_port = 6700;
    bool use_ws = false;
    std::string ws_reverse_api_url = "";
    std::string ws_reverse_event_url = "";
    unsigned long ws_reverse_reconnect_interval = 3000;
    bool use_ws_reverse = false;
    std::string post_url = "";
    std::string access_token = "";
    std::string secret = "";
    std::string post_message_format = "string";
    bool serve_data_files = false;
    std::string update_source = "https://raw.githubusercontent.com/richardchien/coolq-http-api-release/master/";
    std::string update_channel = "stable";
    bool auto_check_update = false;
    size_t thread_pool_size = 4;
    size_t server_thread_pool_size = 1;
};
