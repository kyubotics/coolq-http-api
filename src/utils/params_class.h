// 
// params_class.h : Define Params class,
// which is to help extract parameters from a json object.
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

#include "common.h"

class Params {
public:
    Params() : params_(nullptr) {}
    Params(const json &j) : params_(j) {}

    /**
     * Return std::nullopt if the key does not exist.
     */
    std::optional<json> get(const std::string &key) const;

    template <typename Type>
    std::optional<Type> get(const std::string &key) const {
        auto v = get(key);
        if (v) {
            try {
                return v->get<Type>();
            } catch (std::domain_error &) {
                // type doesn't match
            }
        }
        return std::nullopt;
    }

    std::string get_string(const std::string &key, const std::string &default_val = "") const;
    std::string get_message(const std::string &key = "message",
                            const std::string &auto_escape_key = "auto_escape") const;
    int64_t get_integer(const std::string &key, const int64_t default_val = 0) const;
    bool get_bool(const std::string &key, const bool default_val = false) const;

private:
    json params_;
};
