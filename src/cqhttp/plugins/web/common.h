#pragma once

#include "cqhttp/core/plugin.h"

#include "cqhttp/plugins/web/vendor/simple_web/utility.hpp"

namespace cqhttp::plugins {
    /**
     * Do authorization (check access token),
     * should be called on incomming connection request (http server and websocket server)
     */
    static bool authorize(const std::string &access_token, const SimpleWeb::CaseInsensitiveMultimap &headers,
                          const json &query_args,
                          const std::function<void(SimpleWeb::StatusCode)> on_failed = nullptr) {
        if (access_token.empty()) {
            return true;
        }

        std::string token_given;
        if (const auto headers_it = headers.find("Authorization");
            headers_it != headers.end()
            && (boost::starts_with(headers_it->second, "Token ") || boost::starts_with(headers_it->second, "token "))) {
            token_given = headers_it->second.substr(strlen("Token "));
        } else if (const auto args_it = query_args.find("access_token"); args_it != query_args.end()) {
            token_given = args_it->get<std::string>();
        }

        if (token_given.empty()) {
            if (on_failed) {
                on_failed(SimpleWeb::StatusCode::client_error_unauthorized);
            }
            return false;
        }

        if (token_given != access_token) {
            if (on_failed) {
                on_failed(SimpleWeb::StatusCode::client_error_forbidden);
            }
            return false;
        }

        return true; // token_given == access_token
    }

    static auto fix_server_thread_pool_size(const size_t size) {
        return size > 0 ? size : std::thread::hardware_concurrency() * 2 + 1;
    }
} // namespace cqhttp::plugins
