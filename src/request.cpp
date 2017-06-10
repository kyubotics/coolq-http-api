#include "request.h"

#include <string>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>
#include <jansson/jansson.h>

#include "misc_functions.h"

using namespace std;

static struct cqhttp_result dispatch_request(const struct cqhttp_request& request);

string get_httpd_config_token(); // implemented in appmain.cpp

void cqhttp_main_handler(struct evhttp_request* req, void* _)
{
    enum evhttp_cmd_type method = evhttp_request_get_command(req);
    if (!(method & (EVHTTP_REQ_GET | EVHTTP_REQ_POST)))
    {
        // method not supported
        evhttp_send_reply(req, HTTP_BADMETHOD, "Bad Method", NULL);
        return;
    }

    LOG_D("HTTP请求",
        string("收到 HTTP 请求，请求方式：") + (method == EVHTTP_REQ_GET ? "GET" : "POST") + "，URI：" + evhttp_request_get_uri(req));

    struct evkeyvalq* input_headers = evhttp_request_get_input_headers(req);

    // check token
    string token = get_httpd_config_token();
    if (token != "")
    {
        const char* auth = evhttp_find_header(input_headers, "Authorization");
        if (!auth || "token " + token != auth)
        {
            // invalid token
            LOG_D("HTTP请求", "token 不符，停止响应");
            evhttp_send_reply(req, 401, "Unauthorized", NULL);
            return;
        }
    }

    // initialize args and form
    size_t evkeyvalq_size = sizeof(struct evkeyvalq);
    struct evkeyvalq* args = (struct evkeyvalq *)malloc(evkeyvalq_size);
    memset(args, 0, evkeyvalq_size);
    struct evkeyvalq* form = (struct evkeyvalq *)malloc(evkeyvalq_size);
    memset(form, 0, evkeyvalq_size);

    struct cqhttp_request request;
    request.args = args;
    request.form = form;
    request.json = NULL;

    const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);
    request.path = evhttp_uri_get_path(uri);

    // parse query arguments
    const char* encoded_args_str = evhttp_uri_get_query(uri);
    evhttp_parse_query_str(encoded_args_str ? encoded_args_str : "", args);

    if (method == EVHTTP_REQ_POST)
    {
        // check content type
        string content_type = string(evhttp_find_header(input_headers, "Content-Type"));
        if (content_type == "application/x-www-form-urlencoded" || content_type == "application/json")
        {
            // read request body as string
            struct evbuffer* input_buffer = evhttp_request_get_input_buffer(req);
            size_t length = evbuffer_get_length(input_buffer);
            char* request_body;
            if (length > 0)
            {
                request_body = (char *)malloc(length + 1);
                memcpy(request_body, evbuffer_pullup(input_buffer, -1), length);
                request_body[length] = '\0';
            }
            else
            {
                request_body = (char *)malloc(1);
                request_body[0] = '\0'; // an empty string on heap
            }

            if (content_type == "application/x-www-form-urlencoded")
            {
                // parse form
                evhttp_parse_query_str(request_body, form);
            }
            else
            {
                // parse json
                request.json = json_loads(request_body, 0, NULL);
            }

            free(request_body);
        }
    }

    // dispatch
    struct cqhttp_result result = dispatch_request(request);

    // clear args, form, and json
    evhttp_clear_headers(args);
    free(args);
    evhttp_clear_headers(form);
    free(form);
    if (request.json)
        json_decref(request.json);

    if (result.retcode == CQHTTP_RETCODE_NO_SUCH_API)
    {
        // no such API, should return 404 Not Found
        evhttp_send_reply(req, 404, "Not Found", NULL);
        return;
    }

    // set headers
    struct evkeyvalq* output_headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(output_headers, "Server", CQ_APP_FULLNAME);
    evhttp_add_header(output_headers, "Content-Type", "application/json; charset=UTF-8");
    evhttp_add_header(output_headers, "Connection", "close");

    // write response
    struct evbuffer* buf = evbuffer_new();
    json_t* json = json_pack("{s:s,s:i,s:o?}",
                             "status", result.retcode == CQHTTP_RETCODE_OK ? "ok" : "failed",
                             "retcode", result.retcode,
                             "data", result.data);
    char* json_str = json_dumps(json, JSON_COMPACT);
    json_decref(json);
    evbuffer_add_printf(buf, "%s", json_str);
    free(json_str);
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
    LOG_D("HTTP请求", "响应内容发送完毕");
}

char* cqhttp_get_param(const struct cqhttp_request& request, const char* key)
{
    char* value = NULL;
    const char* encoded_value = evhttp_find_header(request.args, key); // try args
    if (!encoded_value)
        encoded_value = evhttp_find_header(request.form, key); // try form
    if (encoded_value)
        value = evhttp_uridecode(encoded_value, 0, NULL);
    if (!value && request.json)
    {
        // try json
        json_t* json_value = json_object_get(request.json, key);
        if (json_value && json_is_string(json_value))
        {
            const char* const_str = json_string_value(json_value);
            if (const_str)
            {
                size_t len = strlen(const_str);
                value = (char *)malloc(len + 1);
                memcpy(value, const_str, len + 1);
            }
        }
    }
    return value;
}

int64_t cqhttp_get_integer_param(const struct cqhttp_request& request, const char* key, int64_t default_val)
{
    char* str = cqhttp_get_param(request, key);
    int64_t result = default_val;
    if (str)
    {
        if (isnumber(str))
            result = stoll(str);
        free(str);
    }
    else
    {
        // try json
        json_t* json_value = json_object_get(request.json, key);
        if (json_value && json_is_integer(json_value))
            result = json_integer_value(json_value);
    }
    return result;
}

bool cqhttp_get_bool_param(const struct cqhttp_request& request, const char* key, bool default_val)
{
    char* str = cqhttp_get_param(request, key);
    bool result = default_val;
    if (str)
    {
        if (string("true") == str || string("1") == str)
            result = true;
        else if (string("false") == str || string("0") == str)
            result = false;
        free(str);
    }
    else
    {
        // try json
        json_t* json_value = json_object_get(request.json, key);
        if (json_value && json_is_boolean(json_value))
            result = json_boolean_value(json_value);
    }
    return result;
}

cqhttp_request_handler_map request_handler_map;

struct cqhttp_result dispatch_request(const struct cqhttp_request& request)
{
    const char* path_without_slash = &request.path[1];
    if (request_handler_map.find(path_without_slash) != request_handler_map.end())
    {
        // the corresponding handler exists
        LOG_D("HTTP请求", string("找到 handler：") + path_without_slash);
        return request_handler_map[path_without_slash](request);
    }
    else
    {
        // the corresponding handler does not exist
        LOG_D("HTTP请求", string("没有找到 handler：") + path_without_slash);
        struct cqhttp_result result;
        result.retcode = CQHTTP_RETCODE_NO_SUCH_API;
        return result;
    }
}
