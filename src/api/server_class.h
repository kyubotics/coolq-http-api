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

#include "web_server/server_http.hpp"

class ApiServer {
public:
    static ApiServer &instance() {
        static ApiServer instance;
        if (!instance.initiated_) {
            instance.init();
        }
        return instance;
    }

    void init();
    void start(const std::string &host, const unsigned short port);
    void stop();

private:
    ApiServer() {}
    ~ApiServer() { server_.stop(); }
    ApiServer(const ApiServer &) = delete;
    void operator=(const ApiServer &) = delete;

    SimpleWeb::Server<SimpleWeb::HTTP> server_;
    std::thread thread_;
    bool initiated_ = false;
};
