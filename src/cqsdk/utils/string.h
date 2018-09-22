#pragma once

#include "../common.h"

#include <regex>

namespace cq::utils {
    std::string sregex_replace(const std::string &str, const std::regex &re,
                               std::function<std::string(const std::smatch &)> fmt_func);

    enum class Encoding : unsigned {
        // https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756.aspx

        ANSI = 0,
        UTF8 = 65001,
        GB2312 = 936,
        GB18030 = 54936,
    };

    std::string string_encode(const std::string &s, Encoding encoding);
    std::string string_decode(const std::string &b, Encoding encoding);

    std::string string_convert_encoding(const std::string &text, const std::string &from_enc, const std::string &to_enc,
                                        float capability_factor);
    std::string string_encode(const std::string &s, const std::string &encoding, float capability_factor = 2.0f);
    std::string string_decode(const std::string &b, const std::string &encoding, float capability_factor = 2.0f);

    std::string string_to_coolq(const std::string &str);
    std::string string_from_coolq(const std::string &str);

    std::string ws2s(const std::wstring &ws);
    std::wstring s2ws(const std::string &s);
    std::string ansi(const std::string &s);
} // namespace cq::utils

namespace std {
    inline string to_string(const string &val) { return val; }
    inline string to_string(const bool val) { return val ? "true" : "false"; }
} // namespace std
