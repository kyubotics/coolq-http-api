#pragma once

#include "cqhttp/core/common.h"

#include <mutex>

namespace cqhttp::utils::mutex {
    inline void with_unique_lock(std::mutex &m, const std::function<void()> func) {
        std::unique_lock<std::mutex> lock(m);
        func();
    }
} // namespace cqhttp::utils::mutex
