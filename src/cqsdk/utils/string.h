#pragma once

#include "../common.h"

#include <regex>

namespace cq::utils {
    std::string sregex_replace(const std::string &str, const std::regex &re,
                               const std::function<std::string(const std::smatch &)> fmt_func);
    bool is_emoji(const uint32_t codepoint);

    std::string string_convert_encoding(const std::string &text, const std::string &from_enc, const std::string &to_enc,
                                        const float capability_factor);
    std::string string_encode(const std::string &s, const std::string &encoding,
                              const float capability_factor = 2.0f);
    std::string string_decode(const std::string &b, const std::string &encoding,
                              const float capability_factor = 2.0f);

    std::string string_to_coolq(const std::string &str);
    std::string string_from_coolq(const std::string &str);
}
