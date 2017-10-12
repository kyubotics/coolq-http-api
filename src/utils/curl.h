// 
// curl.h : Wrap cURL functions to make life easy.
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

#include <curl/curl.h>
#include <map>

namespace curl {
    struct CaseInsensitiveCompare {
        bool operator()(const std::string &a, const std::string &b) const noexcept {
            return boost::iequals(a, b);
        }
    };

    using Headers = std::map<std::string, std::string, CaseInsensitiveCompare>;
    typedef size_t (*WriteFunction)(char *, size_t, size_t, void *);

    struct Response {
        CURLcode curl_code;
        int32_t status_code;
        Headers headers;
        std::string content_type;
        bytes body;
    };

    enum Method {
        GET,
        POST
    };

    struct Request {
        std::string url;
        Method method = GET;
        Headers headers;
        std::string content_type;
        std::string user_agent;
        bytes body;
        void *write_data = nullptr;
        WriteFunction write_func = nullptr;
        long connect_timeout = 0;
        long timeout = 0;

        Request() {}

        Request(const std::string &url, const std::string &content_type = "", const bytes &body = "") :
            url(url), content_type(content_type), body(body) {}

        Request(const std::string &url, const Headers &headers) : url(url), headers(headers) {}

        Response send() {
            Response response;

            const auto curl = curl_easy_init();
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            if (method == POST) {
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
            }

            auto header_cb = [](char *buf, size_t size, size_t count, void *headers)
            {
                auto line = std::string(buf, count);
                std::vector<std::string> sp;
                boost::split(sp, line, boost::is_any_of(":"));
                if (sp.size() == 2) {
                    boost::trim(sp[0]);
                    boost::trim(sp[1]);
                    (*static_cast<Headers *>(headers))[sp[0]] = sp[1];
                }
                return size * count;
            };
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, static_cast<WriteFunction>(header_cb));

            if (!write_func) {
                write_data = &response.body;
                write_func = [](char *buf, size_t size, size_t count, void *body)
                {
                    *static_cast<bytes *>(body) += std::string(buf, count);
                    return size * count;
                };
            }
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, static_cast<WriteFunction>(write_func));

            if (!content_type.empty()) {
                headers["Content-Type"] = content_type;
            }
            if (!user_agent.empty()) {
                headers["User-Agent"] = user_agent;
            }

            struct curl_slist *chunk = nullptr;
            for (const auto header : headers) {
                chunk = curl_slist_append(chunk, (header.first + ": " + header.second).c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

            if (body.size()) {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            }

            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connect_timeout);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

            response.curl_code = curl_easy_perform(curl);

            if (response.curl_code == CURLE_OK) {
                long status_code;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
                response.status_code = static_cast<int32_t>(status_code);
                if (response.headers.find("Content-Type") != response.headers.end()) {
                    response.content_type = response.headers["Content-Type"];
                }
            } else {
                response.status_code = -1;
            }

            curl_slist_free_all(chunk);
            curl_easy_cleanup(curl);

            return response;
        }

        Response get() {
            method = GET;
            return send();
        }

        Response post() {
            method = POST;
            return send();
        }
    };
}
