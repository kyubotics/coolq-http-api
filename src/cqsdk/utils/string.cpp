#include "./string.h"

#include <iconv.h>
#include <codecvt>

#include "../app.h"
#include "./memory.h"

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

    static shared_ptr<wchar_t> multibyte_to_widechar(const unsigned code_page, const char *multibyte_str) {
        const auto len = MultiByteToWideChar(code_page, 0, multibyte_str, -1, nullptr, 0);
        auto c_wstr_sptr = make_shared_array<wchar_t>(len + 1);
        MultiByteToWideChar(code_page, 0, multibyte_str, -1, c_wstr_sptr.get(), len);
        return c_wstr_sptr;
    }

    static shared_ptr<char> widechar_to_multibyte(const unsigned code_page, const wchar_t *widechar_str) {
        const auto len = WideCharToMultiByte(code_page, 0, widechar_str, -1, nullptr, 0, nullptr, nullptr);
        auto c_str_sptr = make_shared_array<char>(len + 1);
        WideCharToMultiByte(code_page, 0, widechar_str, -1, c_str_sptr.get(), len, nullptr, nullptr);
        return c_str_sptr;
    }

    string string_encode(const string &s, const Encoding encoding) {
        return widechar_to_multibyte(static_cast<unsigned>(encoding), s2ws(s).c_str()).get();
    }

    string string_decode(const string &b, const Encoding encoding) {
        return ws2s(wstring(multibyte_to_widechar(static_cast<unsigned>(encoding), b.c_str()).get()));
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

        auto out_bytes_left =
            static_cast<decltype(in_bytes_left)>(static_cast<double>(in_bytes_left) * capability_factor);
        auto out = new char[out_bytes_left]{0};
        const auto out_begin = out;

        try {
            if (static_cast<size_t>(-1) != iconv(cd, &in, &in_bytes_left, &out, &out_bytes_left)) {
                // successfully converted
                result = out_begin;
            }
        } catch (...) {
        }

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
        // call CoolQ API
        return string_encode(str, "gb18030");
    }

    string string_from_coolq(const string &str) {
        // handle CoolQ event or data
        auto result = string_decode(str, "gb18030");

        if (config.convert_unicode_emoji) {
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

    string ws2s(const wstring &ws) { return wstring_convert<codecvt_utf8<wchar_t>, wchar_t>().to_bytes(ws); }

    wstring s2ws(const string &s) { return wstring_convert<codecvt_utf8<wchar_t>, wchar_t>().from_bytes(s); }

    string ansi(const string &s) { return string_encode(s, Encoding::ANSI); }
} // namespace cq::utils
