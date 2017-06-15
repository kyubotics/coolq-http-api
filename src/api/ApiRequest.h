// 
// ApiRequest.h : Define structure and interface of ApiRequest class.
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

struct evkeyvalq;
struct evhttp_request;

/**
* Wrapper of path, args and form.
*/
struct ApiRequest {
    str path;
    evkeyvalq *args;
    evkeyvalq *form;
    json_t *json;

    str get_str_param(const char *key, const str &default_val = "") const;
    str get_msg_param(const char *key = "message", const char *is_raw_key = "is_raw") const;
    int64_t get_int_param(const char *key, int64_t default_val = 0) const;
    bool get_bool_param(const char *key, bool default_val = false) const;

private:
    /**
    * Get char string parameter from args, form, or json.
    * Every "get_param" call should have a paired "free_param" call to free the memory.
    */
    char *get_param(const char *key) const;
    static void free_param(char *param);
};
