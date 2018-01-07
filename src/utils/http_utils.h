#pragma once

#include "common.h"

std::optional<json> get_remote_json(const std::string &url, const bool use_fake_ua = false,
                                    const std::string &cookies = "");

bool download_remote_file(const std::string &url, const std::string &local_path, const bool use_fake_ua = false);

struct HttpSimpleResponse {
    int status_code = 0;
    std::string body;

    bool ok() const {
        return status_code >= 200 && status_code < 300;
    }
};

HttpSimpleResponse post_json(const std::string &url, const json &payload);
