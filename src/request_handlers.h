#pragma once

#include "app.h"

#include "request.h"

static bool cqhttp_add_request_handler(cqhttp_request_handler_map &m, const char *name, cqhttp_request_handler handler) {
    m[name] = handler;
    return true;
}

extern cqhttp_request_handler_map request_handler_map; // global handler map in request.cpp

#define CQHTTP_REQUEST_HANDLER(handler_name) \
    static void _##handler_name(const struct cqhttp_request &, struct cqhttp_result &); \
    static bool _dummy_##handler_name = cqhttp_add_request_handler(request_handler_map, #handler_name, _##handler_name); \
    static void _##handler_name(const struct cqhttp_request &request, struct cqhttp_result &result)
