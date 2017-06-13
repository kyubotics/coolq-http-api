#pragma once

#include "app.h"

#define CQCODE_ENHANCE_INCOMING 0 // message received
#define CQCODE_ENHANCE_OUTCOMING 1 // message to send out

// 0: full CQ code, 1: function name, 2: params string
const static std::regex CQCODE_REGEX("\\[CQ:([\\._\\-0-9A-Za-z]+?)(?:\\s*\\]|\\s*,\\s*((?:.|\\r?\\n)*?)\\])");

str message_escape(const str &msg);

str message_unescape(const str &msg);

str make_cqcode(const str &function, const str &params_str);

str make_cqcode(const str &function, json_t *params_obj);

str enhance_cqcode(const str &msg, int mode = CQCODE_ENHANCE_OUTCOMING);
