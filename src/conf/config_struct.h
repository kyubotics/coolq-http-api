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
    std::string post_url = "";
    std::string access_token = "";
    std::string secret = "";
    std::string post_message_format = "string";
    bool serve_data_files = false;
    bool auto_check_update = false;
    int thread_pool_size = 4;
};
