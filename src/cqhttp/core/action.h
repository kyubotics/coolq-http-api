#pragma once

#include "cqhttp/core/common.h"

namespace cqhttp::action {
    struct Result {
        enum class Code {
            OK = 0,
            ASYNC = 1,

            // arguments missed or definitely invalid
            DEFAULT_ERROR = 100,
            // the data that CoolQ returns is invalid
            INVALID_DATA = 102,
            // operation failed, often because of insufficient user privilege or filesystem error
            OPERATION_FAILED = 103,
            // thread pool not correctly created
            BAD_THREAD_POOL = 201,

            // retcodes that represent HTTP status codes
            HTTP_BAD_REQUEST = 1400,
            HTTP_UNAUTHORIZED = 1401,
            HTTP_FORBIDDEN = 1403,
            HTTP_NOT_FOUND = 1404,
        };

        Code code = Code::DEFAULT_ERROR;
        json data;
    };

    void to_json(json &j, const Result &r) {
        std::string status;
        switch (r.code) {
        case Result::Code::OK:
            status = "ok";
            break;
        case Result::Code::ASYNC:
            status = "async";
            break;
        default:
            status = "failed";
        }

        j = {
            {"status", status},
            {"retcode", r.code},
            {"data", r.data},
        };
    }
} // namespace cqhttp::action
