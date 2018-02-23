#include "./string.h"

#include <codecvt>
#include <unordered_set>
#include <iconv.h>

#include "../app.h"

using namespace std;

namespace cq::utils {
    string sregex_replace(const string &str, const regex &re, const function<string(const smatch &)> fmt_func) {
        string result;
        auto last_end_pos = 0;
        for (sregex_iterator it(str.begin(), str.end(), re), end; it != end; ++it) {
            result += it->prefix().str() + fmt_func(*it);
            last_end_pos = it->position() + it->length();
        }
        result += str.substr(last_end_pos);
        return result;
    }

    bool is_emoji(const uint32_t codepoint) {
        static unordered_set<uint32_t> emoji_set = {
        #include "../emoji_data.inc"
        };

        return emoji_set.find(codepoint) != emoji_set.end();
    }

    string string_convert_encoding(const string &text, const string &from_enc, const string &to_enc,
                                   const float capability_factor) {
        string result;

        const auto cd = iconv_open(to_enc.c_str(), from_enc.c_str());
        auto in = const_cast<char *>(text.data());
        auto in_bytes_left = text.size();

        if (in_bytes_left == 0) {
            return result;
        }

        auto out_bytes_left = static_cast<decltype(in_bytes_left)>(static_cast<double>(in_bytes_left) *
            capability_factor);
        auto out = new char[out_bytes_left] {0};
        const auto out_begin = out;

        try {
            if (static_cast<size_t>(-1) != iconv(cd, &in, &in_bytes_left, &out, &out_bytes_left)) {
                // successfully converted
                result = out_begin;
            }
        } catch (...) {}

        delete[] out_begin;
        iconv_close(cd);

        return result;
    }

    string string_encode(const string &s, const string &encoding, const float capability_factor) {
        return string_convert_encoding(s, "utf-8", encoding, capability_factor);
    }

    string string_decode(const string &b, const string &encoding, const float capability_factor) {
        return string_convert_encoding(b, encoding, "utf-8", capability_factor);
    }

    string string_to_coolq(const string &str) {
        // call_if_valid CoolQ API

        if (config.convert_unicode_emoji) {
            string processed_str;

            wstring_convert<codecvt_utf8<uint32_t>, uint32_t> uint32_conv;
            auto uint32_str = uint32_conv.from_bytes(str);

            auto append_text = [&](const decltype(uint32_str.cbegin()) &begin,
                                   const decltype(uint32_str.cbegin()) &end) {
                decltype(uint32_str) uint32_part_str(begin, end);
                auto utf8_part_str = uint32_conv.to_bytes(uint32_part_str);
                processed_str += utf8_part_str;
            };

            auto last_it = uint32_str.cbegin();
            for (auto it = uint32_str.cbegin(); it != uint32_str.cend(); ++it) {
                const auto codepoint = *it;
                if (is_emoji(codepoint)) {
                    // is emoji
                    append_text(last_it, it);
                    processed_str += "[CQ:emoji,id=" + to_string(codepoint) + "]";
                    last_it = it + 1;
                }
            }
            append_text(last_it, uint32_str.cend());

            return string_encode(processed_str, "gb18030");
        }

        return string_encode(str, "gb18030");
    }

    string string_from_coolq(const string &str) {
        // handle CoolQ event or data
        auto result = string_decode(str, "gb18030");

        if (config.convert_unicode_emoji) {
            smatch m;

            result = sregex_replace(result, regex(R"(\[CQ:emoji,\s*id=(\d+)\])"), [](const smatch &m) {
                const auto codepoint_str = m.str(1);
                u32string u32_str;

                if (boost::starts_with(codepoint_str, "100000")) {
                    // keycap # to keycap 9
                    const auto codepoint = static_cast<char32_t>(stoul(codepoint_str.substr(strlen("100000"))));
                    u32_str.append({codepoint, 0xFE0F, 0x20E3});
                } else {
                    const auto codepoint = static_cast<char32_t>(stoul(codepoint_str));
                    u32_str.append({codepoint});
                }

                const auto p = reinterpret_cast<const uint32_t *>(u32_str.data());
                wstring_convert<codecvt_utf8<uint32_t>, uint32_t> conv;
                return conv.to_bytes(p, p + u32_str.size());
            });

            // CoolQ sometimes use "#\uFE0F" to represent "#\uFE0F\u20E3"
            // we should convert them into correct emoji codepoints here
            //     \uFE0F == \xef\xb8\x8f
            //     \u20E3 == \xe2\x83\xa3
            result = sregex_replace(result, regex("([#*0-9]\xef\xb8\x8f)(\xe2\x83\xa3)?"), [](const smatch &m) {
                return m.str(1) + "\xe2\x83\xa3";
            });
        }

        return result;
    }
}
