//
// Created by richard on 2017/8/5.
//

#pragma once

#include "common.h"

#include <vector>
#include <functional>

extern std::vector<std::function<void(HMODULE)>> api_initiators;

static bool __add_api_initiator(const std::function<void(HMODULE)> initiator) {
    api_initiators.push_back(initiator);
    return true;
}
