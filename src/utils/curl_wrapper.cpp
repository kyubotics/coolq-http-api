// 
// curl_wrapper.cpp : Implement CURL wrapper.
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

#include "./curl_wrapper.h"

using namespace std;

curl::Response curl::Request::send() {
    Response response;

    const auto curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    if (method == Method::POST) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
    }

    auto header_cb = [](char *buf, size_t size, size_t count, void *headers) {
        auto line = string(buf, count);
        const auto sp = string_split(line, ":\\s*");
        if (sp.size() == 2) {
            (*static_cast<Headers *>(headers))[sp[0]] = sp[1];
        }
        return size * count;
    };
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, static_cast<WriteFunction>(header_cb));

    if (!write_func) {
        write_data = &response.body;
        write_func = [](char *buf, size_t size, size_t count, void *body) {
            *static_cast<bytes *>(body) += string(buf, count);
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
    for (const auto &header : headers) {
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
        response.status_code = static_cast<int>(status_code);
        if (response.headers.find("Content-Type") != response.headers.end()) {
            response.content_type = response.headers["Content-Type"];
        }
        if (response.headers.find("Content-Length") != response.headers.end()) {
            response.content_length = size_t(stoll(response.headers["Content-Length"]));
        }
    } else {
        response.status_code = 0;
    }

    curl_slist_free_all(chunk);
    curl_easy_cleanup(curl);

    return response;
}
