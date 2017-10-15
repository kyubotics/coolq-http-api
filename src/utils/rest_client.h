// 
// rest_client.h : Include cpprestsdk library and add some type aliases.
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
