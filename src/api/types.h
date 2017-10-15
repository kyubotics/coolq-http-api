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
            result = text_to_bool(v.get<std::string>(), default_val);
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
        static const RetCode DEFAULT_ERROR = 100;
        static const RetCode INVALID_DATA = 102; // the data that CoolQ returns is invalid
        static const RetCode ASYNC = 200;
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
