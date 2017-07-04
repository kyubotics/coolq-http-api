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

curl::Response post_json(nlohmann::json json, str post_url) {
    post_url = post_url ? post_url : CQ->config.post_url;
    if (!post_url) {
        return curl::Response();
    }

    auto body = json.dump(0);

    auto req = curl::Request(post_url, "application/json", body);
    if (CQ->config.token != "") {
        req.headers["Authorization"] = "token " + CQ->config.token;
    }
    req.user_agent = CQ_APP_USER_AGENT;
    req.timeout = CQ->config.post_timeout;
    auto resp = req.post();

    auto succeeded = false;
    if (resp.status_code >= 200 && resp.status_code < 300) {
        succeeded = true;
    }

    L.d("HTTP上报", "上报数据到 " + post_url + (succeeded ? " 成功" : " 失败"));

    if (resp.content_length) {
        L.d("HTTP上报", str("收到响应数据 ") + resp.body);
    }

    return resp;
}
