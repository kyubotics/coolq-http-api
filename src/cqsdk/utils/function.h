#pragma once

#include "../common.h"

namespace cq::utils {
    template <typename ReturnType, typename... Params, typename... Args>
    static ReturnType call_if_valid(std::function<ReturnType(Params...)> func, Args &&... args) {
        if (func) {
            return func(std::forward<Args>(args)...);
        }
        return {};
    }

    template <typename... Params, typename... Args>
    static void call_if_valid(std::function<void(Params...)> func, Args &&... args) {
        if (func) {
            func(std::forward<Args>(args)...);
        }
    }
} // namespace cq::utils
