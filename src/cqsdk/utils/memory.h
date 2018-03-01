#pragma once

#include "../common.h"

namespace cq::utils {
    template <typename T>
    static std::shared_ptr<T> make_shared_array(const size_t size) {
        return std::shared_ptr<T>(new T[size], [](T *p) { delete[] p; });
    }
} // namespace cq::utils
