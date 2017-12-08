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

#include <Windows.h>

#include <string>

using Encoding = UINT;

struct Encodings {
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756.aspx

    static const Encoding ANSI = CP_ACP;
    static const Encoding UTF8 = CP_UTF8;
    static const Encoding GB2312 = 936;
    static const Encoding GB18030 = 54936;
};

/**
 * Encode a UTF-8 string into bytes, using the encoding specified.
 */
std::string string_encode(const std::string &s, const Encoding encoding = Encodings::UTF8);

/**
 * Decode bytes into a UTF-8 string, using the encoding specified.
 */
std::string string_decode(const std::string &b, const Encoding encoding = Encodings::UTF8);
