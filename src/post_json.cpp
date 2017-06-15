// 
// post_json.cpp : Implement functions that post json data.
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

#include "post_json.h"

#include "app.h"

#include <curl/curl.h>

#include "helpers.h"

using namespace std;

PostResponse post_json(json_t *json, str post_url) {
    post_url = post_url ? post_url : CQ->config.post_url;
    if (!post_url) {
        return PostResponse();
    }

    auto json_c_str = json_dumps(json, 0);
    auto curl = curl_easy_init();
    PostResponse response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, post_url.c_str());

        stringstream resp_stream;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp_stream);
        auto cb = [](char *buf, size_t size, size_t nmemb, void *stream) {
                    auto tmp = new char[nmemb + 1];
                    memcpy(tmp, buf, nmemb);
                    tmp[nmemb] = '\0';
                    *static_cast<stringstream *>(stream) << tmp;
                    delete[] tmp;
                    return size * nmemb;
                };
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, static_cast<CURLWriteFunctionPtr>(cb));

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_c_str);

        struct curl_slist *chunk = nullptr;
        chunk = curl_slist_append(chunk, "User-Agent: " CQ_APP_FULLNAME);
        chunk = curl_slist_append(chunk, "Content-Type: application/json");
        if (CQ->config.token != "") {
            chunk = curl_slist_append(chunk, ("Authorization: token " + CQ->config.token).c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        auto res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            long status_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
            if (status_code >= 200 && status_code < 300) {
                response.succeeded = true;
                auto resp_json_str = resp_stream.str();
                response.json = json_loads(resp_json_str.c_str(), 0, nullptr);
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(chunk);
    }
    free(json_c_str);
    L.d("HTTP上报", "上报数据到 " + post_url + (response.succeeded ? " 成功" : " 失败"));

    if (response.json) {
        auto tmp = json_dumps(response.json, 0);
        if (tmp) {
            L.d("HTTP上报", str("收到响应数据 ") + tmp);
            free(tmp);
        }
    }

    return response;
}

void release_response(PostResponse &response) {
    json_decref(response.json);
}
