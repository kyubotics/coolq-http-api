#pragma once

#include "app.h"

#define CQCODE_ENHANCE_INCOMING 0
#define CQCODE_ENHANCE_OUTCOMING 1

// 0: full CQ code, 1: function name, 2: params string
const static std::regex CQCODE_REGEX("\\[CQ:([\\._\\-0-9A-Za-z]+?)(?:\\s*\\]|\\s*,\\s*((?:.|\\r?\\n)*?)\\])");

str message_escape(const str &msg);

str message_unescape(const str &msg);

str enhance_cqcode(const str &msg, int mode = CQCODE_ENHANCE_OUTCOMING);
