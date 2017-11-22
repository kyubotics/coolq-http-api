#pragma once

#include "common.h"

std::string hmac_sha1_hex(const std::string &key, const std::string &msg);
std::string md5_hash_hex(const std::string &input);
