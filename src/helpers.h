// 
// helpers.h : Declare helper functions.
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

#pragma once

#include <string>
#include <memory>
#include <optional>
#include <nlohmann/json.hpp>

void string_replace(std::string &str, const std::string &search, const std::string &replace);

std::string ws2s(const std::wstring &ws);
std::wstring s2ws(const std::string &s);
std::string ansi(const std::string &s);

bool to_bool(const std::string &str, const bool default_val);
std::optional<bool> to_bool(const std::string &str);

namespace std {
    inline string to_string(const string &val) {
        return val;
    }

    inline string to_string(const bool val) {
        return val ? "true" : "false";
    }
}

template <typename T>
static std::shared_ptr<T> make_shared_array(size_t size) {
    return std::shared_ptr<T>(new T[size], [](T *p) { delete[] p; });
}

namespace web {
    namespace http {
        class http_request;
    }
}

std::optional<nlohmann::json> get_remote_json(const std::string &url, bool use_fake_ua = false,
                                              const std::string &cookies = "");
std::optional<nlohmann::json> get_remote_json(const std::string &url, const web::http::http_request &request);

bool download_remote_file(const std::string &url, const std::string &local_path, bool use_fake_ua = false);

int message_box(unsigned type, const std::string &text);

std::string hmac_sha1_hex(const std::string &key, const std::string &msg);

bool is_emoji(uint32_t codepoint);

std::string string_to_coolq(const std::string &str);
std::string string_from_coolq(const std::string &str);

unsigned random_int(const unsigned min, const unsigned max);
