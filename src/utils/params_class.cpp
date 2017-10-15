// 
// params_class.cpp : Implement Params class.
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

#include "./params_class.h"

#include "app.h"

#include "message/message_class.h"

using namespace std;

optional<json> Params::get(const string &key) const {
    if (const auto it = params_.find(key); it != params_.end()) {
        return *it;
    }
    return nullopt;
}

string Params::get_string(const string &key, const string &default_val) const {
    auto result = default_val;
    if (auto v_opt = get(key); v_opt && v_opt->is_string()) {
        return v_opt->get<string>();
    }
    return result;
}

string Params::get_message(const string &key, const string &auto_escape_key) const {
    if (auto msg_opt = get(key); msg_opt && !msg_opt->is_null()) {
        if (msg_opt->is_string()) {
            if (get_bool(auto_escape_key, false)) {
                msg_opt = Message::escape(msg_opt->get<string>());
            }
        }
        return Message(msg_opt.value()).process_outward();
    }
    return "";
}

int64_t Params::get_integer(const string &key, const int64_t default_val) const {
    auto result = default_val;
    if (auto v_opt = get(key); v_opt && v_opt->is_string()) {
        try {
            result = stoll(v_opt->get<string>());
        } catch (invalid_argument &) {
            // invalid integer string
        }
    } else if (v_opt->is_number_integer()) {
        result = v_opt->get<int64_t>();
    }
    return result;
}

bool Params::get_bool(const string &key, const bool default_val) const {
    auto result = default_val;
    if (auto v_opt = get(key); v_opt && v_opt->is_string()) {
        result = to_bool(v_opt->get<string>(), default_val);
    } else if (v_opt->is_boolean()) {
        result = v_opt->get<bool>();
    }
    return result;
}
