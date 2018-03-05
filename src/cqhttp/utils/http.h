#pragma once

#include "cqhttp/core/common.h"

#include <map>

namespace cqhttp::utils::http {
    std::optional<json> get_json(const std::string &url, const bool use_fake_ua = false,
                                 const std::string &cookies = "");

    bool download_file(const std::string &url, const std::string &local_path, const bool use_fake_ua = false);

    struct CaseInsensitiveCompare {
        bool operator()(const std::string &a, const std::string &b) const noexcept {
            return _stricmp(a.c_str(), b.c_str()) < 0;
        }
    };

    using Headers = std::map<std::string, std::string, CaseInsensitiveCompare>;

    struct Response {
        int status_code = 0;
        std::string content_type;
        size_t content_length = 0;
        Headers headers;
        std::string body;

        bool ok() const { return status_code >= 200 && status_code < 300; }

        json get_json() const {
            if (content_length == 0) {
                return nullptr;
            }
            if (!json_.is_null()) {
                return json_;
            }
            try {
                return json::parse(body.begin(), body.end());
            } catch (json::parse_error &) {
                return nullptr;
            }
        }

    private:
        mutable json json_;
    };

    Response get(const std::string &url, Headers headers = {});
    Response post(const std::string &url, const std::string &content_type = "text/plain", const std::string &body = "");
    Response post(const std::string &url, Headers headers, const std::string &body = "");
} // namespace cqhttp::utils::http
