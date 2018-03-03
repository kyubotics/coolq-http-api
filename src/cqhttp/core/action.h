#pragma once

#include "cqhttp/core/common.h"

#include "cqhttp/utils/jsonex.h"

namespace cqhttp {
    struct ActionResult {
        struct Codes {
            static const int OK = 0;
            static const int ASYNC = 1;

            static const int DEFAULT_ERROR = 100;    // arguments missed or definitely invalid
            static const int INVALID_DATA = 102;     // the data that CoolQ returns is invalid
            static const int OPERATION_FAILED = 103; // insufficient user privilege or filesystem error
            static const int BAD_THREAD_POOL = 201;  // thread pool not correctly created

            // retcodes that represent HTTP status codes
            static const int HTTP_BAD_REQUEST = 1400;
            static const int HTTP_UNAUTHORIZED = 1401;
            static const int HTTP_FORBIDDEN = 1403;
            static const int HTTP_NOT_FOUND = 1404;
        };

        int code = Codes::DEFAULT_ERROR;
        json data;
    };

    void to_json(json &j, const ActionResult &r) {
        std::string status;
        switch (r.code) {
        case ActionResult::Codes::OK:
            status = "ok";
            break;
        case ActionResult::Codes::ASYNC:
            status = "async";
            break;
        default:
            status = "failed";
            break;
        }

        j = {
            {"status", status},
            {"retcode", r.code},
            {"data", r.data},
        };
    }

    ActionResult call_action(const std::string &action, utils::JsonEx &params);
} // namespace cqhttp
