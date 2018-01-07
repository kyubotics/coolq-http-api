#include "./http_utils.h"

#include "app.h"

#include <regex>
#include <boost/filesystem.hpp>
#include <cpprest/http_client.h>
#undef U  // fix bug in cpprestsdk

#include "utils/curl_wrapper.h"

using namespace std;
namespace fs = boost::filesystem;

namespace http = web::http;
using http::http_exception;
using http::http_headers;
using http::http_request;
using http::http_response;

using http::client::http_client;
using http::client::http_client_config;

#define FAKE_USER_AGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64) " \
    "AppleWebKit/537.36 (KHTML, like Gecko) " \
    "Chrome/56.0.2924.87 Safari/537.36"

static optional<json> get_remote_json_cpprestsdk(const string &url, const bool use_fake_ua, const string &cookies) {
    http_request request(http::methods::GET);
    request.headers().add(L"User-Agent", s2ws(use_fake_ua ? FAKE_USER_AGENT : CQAPP_USER_AGENT));
    request.headers().add(L"Referer", s2ws(url));
    if (!cookies.empty()) {
        request.headers().add(L"Cookie", s2ws(cookies));
    }

    auto task = http_client(s2ws(url))
            .request(request)
            .then([](pplx::task<http_response> task) {
                auto next_task = pplx::task_from_result<string>("");
                try {
                    auto resp = task.get();
                    if (resp.status_code() >= 200 && resp.status_code() < 300) {
                        next_task = resp.extract_utf8string(true);
                    }
                } catch (http_exception &) {
                    // failed to request
                }
                return next_task;
            })
            .then([](string &body) {
                if (smatch m; regex_search(body, m, regex("\\);?\\s*$"))) {
                    // is jsonp
                    if (auto start = body.find("("); start != string::npos) {
                        body = body.substr(start + 1, body.size() - (start + 1) - m.length());
                    }
                }
                if (!body.empty()) {
                    return pplx::task_from_result(make_optional<json>(json::parse(body)));
                    // may throw invalid_argument due to invalid json
                }
                return pplx::task_from_result(optional<json>());
            });

    try {
        return task.get();
    } catch (invalid_argument &) {
        return nullopt;
    }
}

static optional<json> get_remote_json_libcurl(const string &url, const bool use_fake_ua, const string &cookies) {
    auto request = curl::Request(url, curl::Headers{
        {"User-Agent", use_fake_ua ? FAKE_USER_AGENT : CQAPP_USER_AGENT},
        {"Referer", url}
    });
    if (!cookies.empty()) {
        request.headers["Cookie"] = cookies;
    }

    if (const auto response = request.get();
        response.status_code >= 200 && response.status_code < 300) {
        auto body = response.body;
        if (smatch m; regex_search(body, m, regex("\\);?\\s*$"))) {
            // is jsonp
            if (const auto start = body.find("("); start != string::npos) {
                body = body.substr(start + 1, body.size() - (start + 1) - m.length());
            }
        }
        if (!body.empty()) {
            try {
                return json::parse(body);
            } catch (invalid_argument &) {}
        }
    }

    return nullopt;
}

optional<json> get_remote_json(const string &url, const bool use_fake_ua, const string &cookies) {
    if (is_in_wine()) {
        return get_remote_json_libcurl(url, use_fake_ua, cookies);
    }
    return get_remote_json_cpprestsdk(url, use_fake_ua, cookies);
}

static bool download_remote_file_cpprestsdk(const string &url, const string &local_path, const bool use_fake_ua) {
    using concurrency::streams::container_buffer;

    auto succeeded = false;

    auto ansi_local_path = ansi(local_path);

    http_request request(http::methods::GET);
    request.headers().add(L"User-Agent", s2ws(use_fake_ua ? FAKE_USER_AGENT : CQAPP_USER_AGENT));
    request.headers().add(L"Referer", s2ws(url));

    http_client(s2ws(url)).request(request).then([&](http_response response) {
        if (ofstream f(ansi_local_path, ios::out | ios::binary); f.is_open()) {
            auto length = response.headers().content_length();
            decltype(length) read_count = 0;
            auto body_stream = response.body();

            size_t last_read_count = 0;
            do {
                container_buffer<string> buffer;
                const auto count = body_stream.read(buffer, 8192).get();
                read_count += count;
                last_read_count = count;
                f << buffer.collection().substr(0, last_read_count);
            } while (last_read_count > 0);


            if (response.status_code() >= 200 && response.status_code() < 300
                && (length > 0 && read_count == length
                    || length == 0 && read_count > 0)) {
                succeeded = true;
            }
        }
    }).wait();

    if (!succeeded && fs::exists(ansi_local_path)) {
        fs::remove(ansi_local_path);
    }

    return succeeded;
}

static bool download_remote_file_libcurl(const string &url, const string &local_path, const bool use_fake_ua) {
    auto succeeded = false;
    const auto ansi_local_path = ansi(local_path);

    auto request = curl::Request(url, curl::Headers{
        {"User-Agent", use_fake_ua ? FAKE_USER_AGENT : CQAPP_USER_AGENT},
        {"Referer", url}
    });

    struct {
        size_t read_count;
        ofstream file;
    } write_data_wrapper{0, ofstream(ansi_local_path, ios::out | ios::binary)};

    if (write_data_wrapper.file.is_open()) {
        request.write_data = &write_data_wrapper;
        request.write_func = [](char *buf, size_t size, size_t count, void *data) -> size_t {
            auto wrapper = static_cast<decltype(write_data_wrapper) *>(data);
            wrapper->file.write(buf, count);
            wrapper->read_count += size * count;
            return size * count;
        };

        if (const auto response = request.get();
            response.status_code >= 200 && response.status_code < 300
            && (response.content_length > 0 && write_data_wrapper.read_count == response.content_length
                || response.content_length == 0 && write_data_wrapper.read_count > 0)) {
            succeeded = true;
        }

        write_data_wrapper.file.close();
    }

    if (!succeeded && fs::exists(ansi_local_path)) {
        fs::remove(ansi_local_path);
    }

    return succeeded;
}

bool download_remote_file(const string &url, const string &local_path, const bool use_fake_ua) {
    if (is_in_wine()) {
        return download_remote_file_libcurl(url, local_path, use_fake_ua);
    }
    return download_remote_file_cpprestsdk(url, local_path, use_fake_ua);
}

static HttpSimpleResponse post_json_cpprestsdk(const string &url, const json &payload) {
    http_request request(http::methods::POST);
    request.headers().add(L"User-Agent", CQAPP_USER_AGENT);
    request.headers().add(L"Content-Type", L"application/json; charset=UTF-8");
    const auto body = payload.dump();
    request.set_body(payload.dump());
    if (!config.secret.empty()) {
        request.headers().add(L"X-Signature", s2ws("sha1=" + hmac_sha1_hex(config.secret, body)));
    }
    return http_client(s2ws(url))
            .request(request)
            .then([](pplx::task<http_response> task) {
                HttpSimpleResponse result;
                try {
                    auto resp = task.get();
                    result.status_code = resp.status_code();
                    result.body = resp.extract_utf8string(true).get();
                } catch (http_exception &) {
                    // failed to request
                }
                return pplx::task_from_result<HttpSimpleResponse>(result);
            })
            .get();
}

static HttpSimpleResponse post_json_libcurl(const string &url, const json &payload) {
    const auto body = payload.dump();
    auto request = curl::Request(url, "application/json; charset=UTF-8", body);
    request.headers["User-Agent"] = CQAPP_USER_AGENT;
    if (!config.secret.empty()) {
        request.headers["X-Signature"] = "sha1=" + hmac_sha1_hex(config.secret, body);
    }

    const auto response = request.post();

    return {response.status_code, response.body};
}

HttpSimpleResponse post_json(const string &url, const json &payload) {
    if (is_in_wine()) {
        return post_json_libcurl(url, payload);
    }
    return post_json_cpprestsdk(url, payload);
}
