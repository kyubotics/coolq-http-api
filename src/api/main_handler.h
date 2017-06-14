#pragma once

struct evhttp_request;

/**
* Generic handler for evhttp.
*/
void api_main_handler(evhttp_request *req, void *_);
