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

bool isnumber(const std::string &s); // act as "is_positive_integer", actually

bool isfile(const std::string &path);

long long filesize(const std::string &path);

void string_replace(std::string &str, const std::string &search, const std::string &replace);

std::string get_coolq_root();

std::string ansi(const std::string &s);

bool text_to_bool(const std::string &text);

namespace std {
    string to_string(const string &val);
    string to_string(bool val);
}

template <typename T>
static std::shared_ptr<T> make_shared_array(size_t size) {
    return std::shared_ptr<T>(new T[size], [](T *p) { delete[] p; });
}
