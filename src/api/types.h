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

#include "utils/params_class.h"

struct ApiResult {
    using RetCode = int;

    struct RetCodes {
        static const RetCode OK = 0;
        static const RetCode ASYNC = 1;
        static const RetCode DEFAULT_ERROR = 100;
        static const RetCode INVALID_DATA = 102; // the data that CoolQ returns is invalid
        static const RetCode OPERATION_FAILED = 103; // the data that CoolQ returns is invalid
        static const RetCode BAD_THREAD_POOL = 201; // the thread pool isn't properly initiated

        // retcodes that represent HTTP status codes
        static const RetCode HTTP_BAD_REQUEST = 1400;
        static const RetCode HTTP_UNAUTHORIZED = 1401;
        static const RetCode HTTP_FORBIDDEN = 1403;
        static const RetCode HTTP_NOT_FOUND = 1404;
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

using ApiHandler = std::function<void(const Params &, ApiResult &)>;
using ApiHandlerMap = std::unordered_map<std::string, ApiHandler>;
