// 
// helpers.cpp : Implement helper functions.
// 
// Copyright (C) 2017  Richard Chien <richardchienthebest@gmail.com>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

#include "helpers.h"

#include "app.h"

#include <codecvt>
#include <random>
#include <openssl/hmac.h>
#include <boost/compute/detail/lru_cache.hpp>

using namespace std;

void string_replace(string &str, const string &search, const string &replace) {
    if (search.empty())
        return;
    string ws_ret;
    ws_ret.reserve(str.length());
    size_t start_pos = 0, pos;
    while ((pos = str.find(search, start_pos)) != string::npos) {
        ws_ret += str.substr(start_pos, pos - start_pos);
        ws_ret += replace;
        pos += search.length();
        start_pos = pos;
    }
    ws_ret += str.substr(start_pos);
    str.swap(ws_ret);
}

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

bool to_bool(const string &str, const bool default_val) {
    auto result = to_bool(str);
    return result ? result.value() : default_val;
}

optional<bool> to_bool(const string &str) {
    const auto s = boost::algorithm::to_lower_copy(str);
    if (s == "yes" || s == "true" || s == "1") {
        return true;
    }
    if (s == "no" || s == "false" || s == "0") {
        return false;
    }
    return nullopt;
}

int message_box(const unsigned type, const string &text) {
    return MessageBoxW(nullptr,
                       s2ws(text).c_str(),
                       s2ws(CQAPP_NAME).c_str(),
                       type | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);
}

string hmac_sha1_hex(const string &key, const string &msg) {
    unsigned digest_len = 20;
    const auto digest = new unsigned char[digest_len];
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key.c_str(), key.size(), EVP_sha1(), nullptr);
    HMAC_Update(&ctx, reinterpret_cast<const unsigned char *>(msg.c_str()), msg.size());
    HMAC_Final(&ctx, digest, &digest_len);
    HMAC_CTX_cleanup(&ctx);

    stringstream ss;
    for (unsigned i = 0; i < digest_len; ++i) {
        ss << hex << setfill('0') << setw(2) << static_cast<unsigned int>(digest[i]);
    }
    delete[] digest;
    return ss.str();
}

#include "emoji_data.h"

bool is_emoji(const uint32_t codepoint) {
    return emoji_set.find(codepoint) != emoji_set.end();
}

string string_to_coolq(const string &str) {
    // call CoolQ API

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

        return iconv_string_encode(processed_str, "gb18030");
    }

    return iconv_string_encode(str, "gb18030");
}

string string_from_coolq(const string &str) {
    // handle CoolQ event or data
    auto result = iconv_string_decode(str, "gb18030");

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

unsigned random_int(const unsigned min, const unsigned max) {
    default_random_engine engine;
    engine.seed(random_device()());
    const uniform_int_distribution<unsigned> dist(min, max);
    return dist(engine);
}

string data_file_full_path(const string &data_dir, const string &filename) {
    return sdk->directories().coolq() + "data\\" + data_dir + "\\" + filename;
}

bool is_in_wine() {
    static optional<bool> result;

    if (result.has_value()) {
        return result.value();
    }

    auto tmp = false;
    const auto ntdll = GetModuleHandle(L"ntdll.dll");
    if (ntdll) {
        if (GetProcAddress(ntdll, "wine_get_version")) { tmp = true; }
    } else {
        tmp = true;
    }

    result = tmp;
    return tmp;
}
