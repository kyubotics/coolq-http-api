#pragma once

#include "app.h"

enum cqcode_enhance_mode {
    CQCODE_ENHANCE_INCOMING,
    CQCODE_ENHANCE_OUTCOMING
};

std::string message_escape(const std::string &msg);

std::string message_unescape(const std::string &msg);

std::string enhance_cq_code(const std::string &msg, cqcode_enhance_mode mode = CQCODE_ENHANCE_OUTCOMING);
