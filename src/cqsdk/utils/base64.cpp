#include "./base64.h"

#include "./vendor/cpp-base64/base64.h"

namespace cq::utils::base64 {
    std::string encode(const unsigned char *bytes, const unsigned int len) { return base64_encode(bytes, len); }
    std::string decode(const std::string &str) { return base64_decode(str); }
} // namespace cq::utils::base64
