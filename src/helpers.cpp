// 
// helpers.cpp : Implement helper functions.
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

#include "helpers.h"

#include "app.h"

#include <openssl/hmac.h>
#include <boost/filesystem.hpp>
#include <cpprest/filestream.h>

#include "utils/rest_client.h"

using namespace std;

void string_replace(string &str, const string &search, const string &replace) {
    if (search.empty())
        return;
    string ws_ret;
    ws_ret.reserve(str.length());
    size_t start_pos = 0, pos;
    while ((pos = str.find(search, start_pos)) != string::npos) {
        ws_ret += str.substr(start_pos, pos - start_pos);
        ws_ret += replace;
        pos += search.length();
        start_pos = pos;
    }
    ws_ret += str.substr(start_pos);
    str.swap(ws_ret); // faster than str = wsRet;
}

string ansi(const string &s) {
    return string_encode(s, Encodings::ANSI);
}

bool to_bool(const string &str, const bool default_val) {
    auto result = to_bool(str);
    return result ? result.value() : default_val;
}

optional<bool> to_bool(const string &str) {
    const auto s = boost::algorithm::to_lower_copy(str);
    if (s == "yes" || s == "true" || s == "1") {
        return true;
    }
    if (s == "no" || s == "false" || s == "0") {
        return false;
    }
    return nullopt;
}

namespace std {
    string to_string(const string &val) {
        return val;
    }

    string to_string(bool val) {
        return val ? "true" : "false";
    }
}

optional<json> get_remote_json(const string &url) {
    http_request request(http::methods::GET);
    request.headers().add(L"User-Agent", CQAPP_USER_AGENT);
    auto task = http_client(s2ws(url))
            .request(request)
            .then([](pplx::task<http_response> task) {
                auto next_task = pplx::task_from_result<string>("");
                try {
                    auto resp = task.get();
                    if (resp.status_code() == 200) {
                        next_task = resp.extract_utf8string(true);
                    }
                } catch (http_exception &) {
                    // failed to request
                }
                return next_task;
            })
            .then([](string &body) {
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

bool download_remote_file(const string &url, const string &local_path, bool use_fake_ua) {
    using concurrency::streams::ostream;
    using concurrency::streams::fstream;

    optional<ostream> file_stream;
    auto succeeded = false;
    fstream::open_ostream(s2ws(local_path)).then([&](ostream out_file) {
        file_stream = out_file;

        http_client client(s2ws(url));
        http_request request(http::methods::GET);

        string user_agent(CQAPP_USER_AGENT);
        if (use_fake_ua) {
            user_agent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
                    "AppleWebKit/537.36 (KHTML, like Gecko) "
                    "Chrome/56.0.2924.87 Safari/537.36";
        }
        request.headers().add(L"User-Agent", s2ws(user_agent));
        request.headers().add(L"Referer", s2ws(url));

        return client.request(request);
    }).then([&](http_response resp) {
        if (resp.status_code() == 200) {
            // we can assume here that the request is succeeded
            return resp.body().read_to_end(file_stream->streambuf());
        }
        return pplx::task_from_result<size_t>(0);
    }).then([&](size_t size) {
        if (size > 0) {
            // download succeeded
            succeeded = true;
        }
        return file_stream->close();
    }).wait();

    return succeeded;
}

int message_box(unsigned type, const string &text) {
    return MessageBoxW(nullptr,
                       s2ws(text).c_str(),
                       s2ws(CQAPP_NAME).c_str(),
                       type | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);
}

string hmac_sha1_hex(string key, string msg) {
    unsigned digest_len = 20;
    const auto digest = new unsigned char[digest_len];
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key.c_str(), key.size(), EVP_sha1(), nullptr);
    HMAC_Update(&ctx, reinterpret_cast<const unsigned char *>(msg.c_str()), msg.size());
    HMAC_Final(&ctx, digest, &digest_len);
    HMAC_CTX_cleanup(&ctx);

    stringstream ss;
    for (unsigned i = 0; i < digest_len; ++i) {
        ss << hex << setfill('0') << setw(2) << static_cast<unsigned int>(digest[i]);
    }
    delete[] digest;
    return ss.str();
}
