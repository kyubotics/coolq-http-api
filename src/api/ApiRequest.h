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
