#pragma once

#include "../common.h"

namespace cq::utils::base64 {
    std::string encode(const unsigned char *bytes, unsigned int len);
    std::string decode(const std::string &str);
} // namespace cq::utils::base64
