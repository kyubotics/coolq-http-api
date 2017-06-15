// 
// types.h : Define types that used is API module.
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

#include "ApiRequest.h"

struct ApiRetCode {
    const static int OK = 0;
    const static int DEFAULT_ERROR = 100;
    const static int NO_SUCH_API = 101; // will be converted to 404 Not Found http error
    const static int INVALID_DATA = 102; // the data that CoolQ returns is invalid
};

struct ApiResult {
    ApiResult() : retcode(ApiRetCode::DEFAULT_ERROR), data(nullptr) {};
    int retcode; // succeeded: 0, lack of parameters or invalid ones: 1xx, CQ error code: -11, -23, etc... (< 0)
    json_t *data;
};

typedef void(*ApiHandler)(const ApiRequest &request, struct ApiResult &result);

struct CharStrComparator {
    bool operator()(char const *a, char const *b) const {
        return strcmp(a, b) < 0;
    }
};

#include <map>

typedef std::map<const char *, ApiHandler, CharStrComparator> ApiHandlerMap;
