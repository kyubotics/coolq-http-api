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
#include <regex>
#include <memory>
#include <optional>
#include <nlohmann/json.hpp>

void string_replace(std::string &str, const std::string &search, const std::string &replace);
std::string sregex_replace(const std::string &str, const std::regex &re,
                           const std::function<std::string(const std::smatch &)> fmt_func);

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
static std::shared_ptr<T> make_shared_array(const size_t size) {
    return std::shared_ptr<T>(new T[size], [](T *p) { delete[] p; });
}

int message_box(const unsigned type, const std::string &text);

std::string hmac_sha1_hex(const std::string &key, const std::string &msg);

bool is_emoji(const uint32_t codepoint);

std::string string_to_coolq(const std::string &str);
std::string string_from_coolq(const std::string &str);

unsigned random_int(const unsigned min, const unsigned max);

std::string data_file_full_path(const std::string &data_dir, const std::string &filename);

bool is_in_wine();
