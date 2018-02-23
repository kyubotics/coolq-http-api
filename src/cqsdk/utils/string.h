#pragma once

#include "../common.h"

#include <regex>

namespace cq::utils {
    std::string sregex_replace(const std::string &str, const std::regex &re,
                               const std::function<std::string(const std::smatch &)> fmt_func);
    bool is_emoji(const uint32_t codepoint);

    using Encoding = unsigned;

    struct Encodings {
        // https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756.aspx

        static const Encoding ANSI = 0;
        static const Encoding UTF8 = 65001;
        static const Encoding GB2312 = 936;
        static const Encoding GB18030 = 54936;
    };

    std::string string_encode(const std::string &s, const Encoding encoding);
    std::string string_decode(const std::string &b, const Encoding encoding);

    std::string string_convert_encoding(const std::string &text, const std::string &from_enc, const std::string &to_enc,
                                        const float capability_factor);
    std::string string_encode(const std::string &s, const std::string &encoding,
                              const float capability_factor = 2.0f);
    std::string string_decode(const std::string &b, const std::string &encoding,
                              const float capability_factor = 2.0f);

    std::string string_to_coolq(const std::string &str);
    std::string string_from_coolq(const std::string &str);

    std::string ws2s(const std::wstring &ws);
    std::wstring s2ws(const std::string &s);
    std::string ansi(const std::string &s);
}
