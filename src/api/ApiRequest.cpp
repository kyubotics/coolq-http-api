// 
// ApiRequest.cpp : Implement methods of ApiRequest.
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

#include "ApiRequest.h"

#include "app.h"

#include <event2/http.h>
#include <event2/buffer.h>

#include "Message.h"
#include "cqcode.h"
#include "helpers.h"

using namespace std;

char *ApiRequest::get_param(const char *key) const {
    char *value = nullptr;
    const char *encoded_value = nullptr;
    if (this->args) {
        encoded_value = evhttp_find_header(this->args, key); // try args
    }
    if (!encoded_value && this->form) {
        encoded_value = evhttp_find_header(this->form, key); // try form
    }
    if (encoded_value) {
        value = evhttp_uridecode(encoded_value, 0, nullptr);
    }
    if (!value && this->json) {
        // try json
        auto json_value = json_object_get(this->json, key);
        if (json_is_string(json_value)) {
            auto const_str = json_string_value(json_value);
            if (const_str) {
                auto len = strlen(const_str);
                value = static_cast<char *>(malloc(len + 1));
                memcpy(value, const_str, len + 1);
            }
        }
    }
    return value;
}

void ApiRequest::free_param(char *param) {
    if (param) {
        free(param); // the param char string is "malloc"ed, so we "free" it here
    }
}

str ApiRequest::get_str_param(const char *key, const str &default_val) const {
    auto value = this->get_param(key);
    if (value) {
        auto tmp = str(value);
        this->free_param(value);
        return tmp;
    }
    return default_val;
}

str ApiRequest::get_msg_param(const char *key, const char *is_raw_key) const {
    auto msg_str = this->get_str_param(key, "");
    if (msg_str) {
        auto is_raw = this->get_bool_param(is_raw_key, false);
        return Message(is_raw ? message_escape(msg_str) : msg_str).process_outcoming();
    }
    return Message(json_object_get(this->json, key)).process_outcoming();
}

int64_t ApiRequest::get_int_param(const char *key, int64_t default_val) const {
    auto str = this->get_param(key);
    auto result = default_val;
    if (str) {
        if (isnumber(str)) {
            result = stoll(str);
        }
        this->free_param(str);
    } else {
        // try json
        auto json_value = json_object_get(this->json, key);
        if (json_is_integer(json_value)) {
            result = json_integer_value(json_value);
        }
    }
    return result;
}

bool ApiRequest::get_bool_param(const char *key, bool default_val) const {
    auto str = this->get_param(key);
    auto result = default_val;
    if (str) {
        if (string("true") == str || string("1") == str) {
            result = true;
        } else if (string("false") == str || string("0") == str) {
            result = false;
        }
        this->free_param(str);
    } else {
        // try json
        auto json_value = json_object_get(this->json, key);
        if (json_is_boolean(json_value)) {
            result = json_boolean_value(json_value);
        }
    }
    return result;
}
