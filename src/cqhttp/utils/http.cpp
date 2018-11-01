#include "./http.h"

#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <regex>

using namespace std;
namespace fs = std::filesystem;

#define FAKE_USER_AGENT                          \
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) " \
    "AppleWebKit/537.36 (KHTML, like Gecko) "    \
    "Chrome/56.0.2924.87 Safari/537.36"

namespace cqhttp::utils::http::curl {
    typedef size_t (*WriteFunction)(char *, size_t, size_t, void *);

    struct Response : http::Response {
        int curl_code = -1;
    };

    enum class Method { GET, POST };

    struct Request {
        string url;
        Method method = Method::GET;
        Headers headers;
        string content_type;
        string user_agent;
        string body;
        void *write_data = nullptr;
        WriteFunction write_func = nullptr;
        long connect_timeout = 0;
        long timeout = 0;

        Request() = default;

        Request(const string &url, const string &content_type = "", const string &body = "")
            : url(url), content_type(content_type), body(body) {}

        Request(const string &url, const Headers &headers, const string &body = "")
            : url(url), headers(headers), body(body) {}

        Response Request::send() {
            Response response;

            const auto curl = curl_easy_init();
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            if (method == Method::POST) {
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
            }

            const auto header_cb = [](char *buf, size_t size, size_t count, void *headers) {
                auto line = string(buf, count);
                const auto sep_pos = line.find(":");
                if (sep_pos != string::npos) {
                    const auto k = line.substr(0, sep_pos);
                    auto v = line.substr(sep_pos + 1);
                    boost::algorithm::trim_left(v);
                    (*static_cast<Headers *>(headers))[k] = v;
                }
                return size * count;
            };
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, static_cast<WriteFunction>(header_cb));

            if (!write_func) {
                write_data = &response.body;
                write_func = [](char *buf, size_t size, size_t count, void *body) {
                    *static_cast<string *>(body) += string(buf, count);
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

            if (!body.empty()) {
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

        Response get() {
            method = Method::GET;
            return send();
        }

        Response post() {
            method = Method::POST;
            return send();
        }
    };
} // namespace cqhttp::utils::http::curl

namespace cqhttp::utils::http {
    optional<json> get_json(const string &url, const bool use_fake_ua, const string &cookies) {
        auto request =
            curl::Request(url, {{"User-Agent", use_fake_ua ? FAKE_USER_AGENT : CQHTTP_USER_AGENT}, {"Referer", url}});
        if (!cookies.empty()) {
            request.headers["Cookie"] = cookies;
        }

        if (const auto response = request.get(); response.status_code >= 200 && response.status_code < 300) {
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
                } catch (json::parse_error &) {
                }
            }
        }

        return nullopt;
    }

    bool download_file(const string &url, const string &local_path, const bool use_fake_ua) {
        auto succeeded = false;
        const auto ansi_local_path = ansi(local_path);

        auto request =
            curl::Request(url, {{"User-Agent", use_fake_ua ? FAKE_USER_AGENT : CQHTTP_USER_AGENT}, {"Referer", url}});

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

            const auto response = request.get();
            write_data_wrapper.file.close();

            if (response.status_code >= 200 && response.status_code < 300
                && (response.content_length > 0 && write_data_wrapper.read_count == response.content_length
                    || response.content_length == 0 && write_data_wrapper.read_count > 0)) {
                logging::debug(u8"下载文件",
                               local_path + "\ndownloaded size: " + to_string(write_data_wrapper.read_count)
                                   + "\ncontent length: " + to_string(response.content_length)
                                   + "\nfile size: " + to_string(fs::file_size(ansi_local_path)));
                succeeded = true;
            }
        }

        if (!succeeded && fs::exists(ansi_local_path)) {
            fs::remove(ansi_local_path);
        }

        return succeeded;
    }

    static void fix_headers(Headers &headers) {
        if (headers.find("User-Agent") == headers.end()) {
            headers["User-Agent"] = CQHTTP_USER_AGENT;
        }
    }

    Response get(const string &url, Headers headers, const long timeout) {
        fix_headers(headers);
        auto request = curl::Request(url, headers);
        request.timeout = timeout;
        const auto res = request.get();
        return static_cast<Response>(res);
    }

    Response post(const string &url, const string &body, const string &content_type, const long timeout) {
        return post(url, body, {{"Content-Type", content_type}}, timeout);
    }

    Response post(const string &url, const std::string &body, Headers headers, const long timeout) {
        fix_headers(headers);
        auto request = curl::Request(url, headers, body);
        request.timeout = timeout;
        const auto res = request.post();
        return static_cast<Response>(res);
    }
} // namespace cqhttp::utils::http
