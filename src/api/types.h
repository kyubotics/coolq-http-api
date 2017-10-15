// 
// types.h : Define types used in API module.
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
#include "message/message_class.h"

struct ApiParams {
    ApiParams(const json &j) : params_(j) {}

    json get(const std::string &key) const {
        if (const auto it = params_.find(key); it != params_.end()) {
            return *it;
        }
        return json(nullptr);
    }

    std::string get_string(const std::string &key, const std::string &default_val = "") const {
        auto result = default_val;
        if (auto v = get(key); v.is_string()) {
            return v.get<std::string>();
        }
        return result;
    }

    std::string get_message(const std::string &key = "message",
                            const std::string &auto_escape_key = "auto_escape") const {
        if (auto msg = get(key); !msg.is_null()) {
            if (msg.is_string()) {
                if (get_bool(auto_escape_key)) {
                    msg = Message::escape(msg.get<std::string>());
                }
            }
            return Message(msg).process_outward();
        }
        return "";
    }

    int64_t get_integer(const std::string &key, const int64_t default_val = 0) const {
        auto result = default_val;
        if (auto v = get(key); v.is_string()) {
            try {
                result = stoll(v.get<std::string>());
            } catch (std::invalid_argument &) {
                // invalid integer string
            }
        } else if (v.is_number_integer()) {
            result = v.get<int64_t>();
        }
        return result;
    }

    bool get_bool(const std::string &key, const bool default_val = false) const {
        auto result = default_val;
        if (auto v = get(key); v.is_string()) {
            result = to_bool(v.get<std::string>(), default_val);
        } else if (v.is_boolean()) {
            result = v.get<bool>();
        }
        return result;
    }

private:
    json params_;
};

struct ApiResult {
    using RetCode = int;

    struct RetCodes {
        static const RetCode OK = 0;
        static const RetCode ASYNC = 1;
        static const RetCode DEFAULT_ERROR = 100;
        static const RetCode INVALID_DATA = 102; // the data that CoolQ returns is invalid
        static const RetCode BAD_THREAD_POOL = 201; // the thread pool isn't properly initiated
    };

    RetCode retcode; // succeeded: 0, lack of parameters or invalid ones: 1xx, CQ error code: -11, -23, etc... (< 0)
    json data;

    ApiResult() : retcode(RetCodes::DEFAULT_ERROR) {}

    json json() const {
        std::string status;
        switch (retcode) {
        case RetCodes::OK:
            status = "ok";
            break;
        case RetCodes::ASYNC:
            status = "async";
            break;
        default:
            status = "failed";
        }
        return {
            {"status", status},
            {"retcode", retcode},
            {"data", data}
        };
    }
};

using ApiHandler = std::function<void(const ApiParams &, ApiResult &)>;
using ApiHandlerMap = std::map<std::string, ApiHandler>;
