// 
// curl_wrapper.h : Wrap cURL functions to make life easy.
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

#include <map>

namespace curl {
    struct CaseInsensitiveCompare {
        bool operator()(const std::string &a, const std::string &b) const noexcept {
            return _stricmp(a.c_str(), b.c_str()) < 0;
        }
    };

    using Headers = std::map<std::string, std::string, CaseInsensitiveCompare>;
    typedef size_t (*WriteFunction)(char *, size_t, size_t, void *);

    struct Response {
        int curl_code;
        int status_code = 0;
        Headers headers;
        std::string content_type;
        size_t content_length = 0;
        bytes body;

        json get_json() {
            if (content_length == 0) {
                return nullptr;
            }
            if (!json_.is_null()) {
                return json_;
            }
            return json::parse(body.begin(), body.end());
        }

    private:
        json json_;
    };

    enum class Method {
        GET,
        POST
    };

    struct Request {
        std::string url;
        Method method = Method::GET;
        Headers headers;
        std::string content_type;
        std::string user_agent;
        bytes body;
        void *write_data = nullptr;
        WriteFunction write_func = nullptr;
        long connect_timeout = 0;
        long timeout = 0;

        Request() {}

        Request(const std::string &url, const std::string &content_type = "", const bytes &body = "") :
            url(url), content_type(content_type), body(body) {}

        Request(const std::string &url, const Headers &headers) : url(url), headers(headers) {}

        Response send();

        Response get() {
            method = Method::GET;
            return send();
        }

        Response post() {
            method = Method::POST;
            return send();
        }
    };
}
