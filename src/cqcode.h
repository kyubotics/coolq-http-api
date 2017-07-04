// 
// cqcode.h : Declare functions and global constant about CQ code.
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

#pragma once

#include "app.h"

#define CQCODE_ENHANCE_INCOMING 0 // message received
#define CQCODE_ENHANCE_OUTCOMING 1 // message to send out

// 0: full CQ code, 1: function name, 2: params string
const static std::regex CQCODE_REGEX(R"(\[CQ:([0-9A-Za-z]+?)(?:\s*\]|,((?:.|\r|\n)*?)\]))");

str message_escape(const str &msg);

str message_unescape(const str &msg);

str make_cqcode(const str &function, const str &params_str);

str make_cqcode(const str &function, json_t *params_obj);

str enhance_cqcode(const str &msg, int mode = CQCODE_ENHANCE_OUTCOMING);
