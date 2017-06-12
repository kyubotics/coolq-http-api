#include "post_json.h"

#include "app.h"

#include <jansson/jansson.h>
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

int release_response(PostResponse &response) {
    auto block = json_is_true(json_object_get(response.json, "block"));
    json_decref(response.json);
    return block ? EVENT_BLOCK : EVENT_IGNORE;
}
