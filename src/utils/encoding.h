// 
// encoding.h : Handle string encodings.
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

#include <string>

using bytes = std::string;
using Encoding = unsigned;

struct Encodings {
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756.aspx

    static const Encoding ANSI = 0;
    static const Encoding UTF8 = 65001;
    static const Encoding GB2312 = 936;
    static const Encoding GB18030 = 54936;
};

std::string ws2s(const std::wstring &ws);
std::wstring s2ws(const std::string &s);
std::string ansi(const std::string &s);

/**
 * Encode a UTF-8 string into bytes, using the encoding specified.
 */
bytes string_encode(const std::string &s, const Encoding encoding = Encodings::UTF8);

/**
 * Decode bytes into a UTF-8 string, using the encoding specified.
 */
std::string string_decode(const bytes &b, const Encoding encoding = Encodings::UTF8);

bytes iconv_string_encode(const std::string &s, const std::string &encoding,
                          const float capability_factor = 2.0f);
std::string iconv_string_decode(const bytes &b, const std::string &encoding,
                                const float capability_factor = 2.0f);
