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
#include <memory>

#include "helpers.h"

using bytes = std::string;

static std::shared_ptr<wchar_t> multibyte_to_widechar(const int code_page, const char *multibyte_str) {
    const auto len = MultiByteToWideChar(code_page, 0, multibyte_str, -1, nullptr, 0);
    auto c_wstr_sptr = make_shared_array<wchar_t>(len + 1);
    MultiByteToWideChar(code_page, 0, multibyte_str, -1, c_wstr_sptr.get(), len);
    return c_wstr_sptr;
}

static std::shared_ptr<char> widechar_to_multibyte(const int code_page, const wchar_t *widechar_str) {
    const auto len = WideCharToMultiByte(code_page, 0, widechar_str, -1, nullptr, 0, nullptr, nullptr);
    auto c_str_sptr = make_shared_array<char>(len + 1);
    WideCharToMultiByte(code_page, 0, widechar_str, -1, c_str_sptr.get(), len, nullptr, nullptr);
    return c_str_sptr;
}

using Encoding = UINT;

struct Encodings {
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756.aspx

    static const Encoding ANSI = CP_ACP;
    static const Encoding UTF7 = CP_UTF7;
    static const Encoding UTF8 = CP_UTF8;
    static const Encoding SHIFT_JIS = 932;
    static const Encoding GB2312 = 936;
    static const Encoding KS_C_5601_1987 = 949;
    static const Encoding BIG5 = 950;
    static const Encoding JOHAB = 1361;
    static const Encoding EUC_JP = 51932;
    static const Encoding EUC_CN = 51936;
    static const Encoding EUC_KR = 51949;
    static const Encoding EUC_TW = 51950;
    static const Encoding GB18030 = 54936;
    static const Encoding GBK = GB18030;
};

/**
 * Encode a UTF-8 string into bytes, using the encoding specified.
 */
static bytes string_encode(const std::string &s, const Encoding encoding = Encodings::UTF8) {
    return widechar_to_multibyte(encoding, s2ws(s).c_str()).get();
}

/**
 * Decode bytes into a UTF-8 string, using the encoding specified.
 */
static std::string string_decode(const bytes &b, const Encoding encoding = Encodings::UTF8) {
    return ws2s(std::wstring(multibyte_to_widechar(encoding, b.c_str()).get()));
}
