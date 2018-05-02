#pragma once

#include "cqhttp/core/common.h"

namespace cqhttp::utils::crypt {
    std::string hmac_sha1_hex(const std::string &key, const std::string &msg);
}
