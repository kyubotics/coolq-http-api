#pragma once

#include "common.h"

struct PostResponse {
    bool succeeded; // post event succeeded or not (the server returning 2xx means success)
    json_t *json; // response json of the post request, NULL if response body is empty
    PostResponse() : succeeded(false), json(nullptr) {}
};

PostResponse post_json(json_t *json, str post_url = "");
void release_response(PostResponse &response);
