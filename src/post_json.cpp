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

#include "curl_wrapper.h"

using namespace std;

PostResponse post_json(json_t *json, str post_url) {
    post_url = post_url ? post_url : CQ->config.post_url;
    if (!post_url) {
        return PostResponse();
    }

    auto json_c_str = json_dumps(json, 0);

    PostResponse response;

    if (post_url.startswith("https://maker.ifttt.com/trigger/")) {
        // convert to the format that IFTTT can recognize
        auto ifttt_json = json_pack("{s:s}", "value1", json_c_str);
        free(json_c_str);
        json_c_str = json_dumps(ifttt_json, 0);
    }

    auto req = curl::Request(post_url, "application/json", json_c_str);
    if (CQ->config.token != "") {
        req.headers["Authorization"] = "token " + CQ->config.token;
    }
    req.user_agent = CQ_APP_USER_AGENT;
    req.timeout = CQ->config.post_timeout;
    auto resp = req.post();
    if (resp.status_code >= 200 && resp.status_code < 300) {
        response.succeeded = true;
        response.json = json_loads(resp.body.c_str(), 0, nullptr);
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
