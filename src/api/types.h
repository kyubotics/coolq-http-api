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
    int retcode; // succeeded: 0, lack of parameters or invalid ones: 1, CQ error code: -11, -23, etc... (< 0)
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
