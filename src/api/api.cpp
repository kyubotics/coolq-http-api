#include "./api.h"

using namespace std;

extern ApiHandlerMap api_handlers; // defined in handlers.cpp

void invoke_api(const string &action, const Params &params, ApiResult &result) {
    if (const auto it = api_handlers.find(action); it != api_handlers.end()) {
        it->second(params, result);
    } else {
        throw invalid_argument("there is no api handler matching the given \"action\"");
    }
}

void invoke_api(const string &action, const Params &params) {
    static ApiResult result;
    invoke_api(action, params, result);
}
