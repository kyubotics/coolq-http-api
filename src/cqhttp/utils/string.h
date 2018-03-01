#pragma once

#include "cqhttp/core/common.h"

namespace cqhttp::utils {
    inline std::optional<bool> to_bool(const std::string &str) {
        const auto s = boost::algorithm::to_lower_copy(str);
        if (s == "yes" || s == "true" || s == "1") {
            return true;
        }
        if (s == "no" || s == "false" || s == "0") {
            return false;
        }
        return std::nullopt;
    }

    inline bool to_bool(const std::string &str, const bool default_val) {
        auto result = to_bool(str);
        return result ? result.value() : default_val;
    }
} // namespace cqhttp::utils
