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

#include <boost/filesystem.hpp>

using namespace std;

bool isfile(const string &path) {
    //struct stat st;
    //if (stat(ansi(path).c_str(), &st) < 0) {
    //    return false;
    //}
    //return (st.st_mode & S_IFMT) != S_IFDIR;

    return boost::filesystem::is_regular_file(ansi(path));
}

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
    str.swap(ws_ret); // faster than str = wsRet;
}

string ansi(const string &s) {
    return string_encode(s, Encodings::ANSI);
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

namespace std {
    string to_string(const string &val) {
        return val;
    }

    string to_string(bool val) {
        return val ? "true" : "false";
    }
}
