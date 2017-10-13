#pragma once

#include <cpprest/http_client.h>
#undef U  // fix bug in cpprestsdk

namespace http {
    using namespace web::http;
}

using http::http_exception;
using http::http_headers;
using http::http_request;
using http::http_response;

using http::client::http_client;
using http::client::http_client_config;
