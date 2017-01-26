#pragma once

#include "stdafx.h"

struct evkeyvalq;
struct evhttp_request;
typedef struct json_t json_t;

/**
 * Wrapper of path, args and form.
 */
struct cqhttp_request
{
    const char *path;
    const struct evkeyvalq *args;
    const struct evkeyvalq *form;
};

/**
 * Generic handler for evhttp.
 */
void cqhttp_main_handler(struct evhttp_request *req, void *_);

enum cqhttp_status
{
    CQHTTP_STATUS_OK = 0,
    CQHTTP_STATUS_FAILED
};

static const char *cqhttp_status_strings[] = {"ok", "failed"};

struct cqhttp_result
{
    cqhttp_result() : status(CQHTTP_STATUS_OK), data(NULL){};
    enum cqhttp_status status;
    json_t *data;
};

/**
 * Get char string parameter from url arguments, and if not exists and the http method is POST, try form.
 * The return value should be freed by caller.
 */
char *cqhttp_get_param(const struct cqhttp_request &request, const char *key);

/**
 * Like cqhttp_get_param, but parse integer if possible. Return default_val if it's not a number.
 */
int64_t cqhttp_get_integer_param(const struct cqhttp_request &request, const char *key, int64_t default_val);

/**
 * Like cqhttp_get_param, but parse bool if possible. Return default_val if it's not a number.
 */
bool cqhttp_get_bool_param(const struct cqhttp_request &request, const char *key, bool default_val);

typedef struct cqhttp_result (*cqhttp_request_handler)(const struct cqhttp_request &request);

#include <cstring>

struct c_str_comparator
{
    bool operator()(char const *a, char const *b)
    {
        return strcmp(a, b) < 0;
    }
};

#include <map>

typedef std::map<const char *, cqhttp_request_handler, c_str_comparator> cqhttp_request_handler_map;
