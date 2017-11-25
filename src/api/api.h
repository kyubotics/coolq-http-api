#pragma once

#include "common.h"

#include "./types.h"

void invoke_api(const std::string &action, const Params &params, ApiResult &result);
void invoke_api(const std::string &action, const Params &params = {});
