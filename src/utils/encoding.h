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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>
#include <memory>

#include "helpers.h"

using bytes = std::string;

static auto multibyte_to_widechar(const int code_page, const char *multibyte_str) {
    const auto len = MultiByteToWideChar(code_page, 0, multibyte_str, -1, nullptr, 0);
    auto c_wstr_sptr = make_shared_array<wchar_t>(len + 1);
    MultiByteToWideChar(code_page, 0, multibyte_str, -1, c_wstr_sptr.get(), len);
    return c_wstr_sptr;
}

static auto widechar_to_multibyte(const int code_page, const wchar_t *widechar_str) {
    const auto len = WideCharToMultiByte(code_page, 0, widechar_str, -1, nullptr, 0, nullptr, nullptr);
    auto c_str_sptr = make_shared_array<char>(len + 1);
    WideCharToMultiByte(code_page, 0, widechar_str, -1, c_str_sptr.get(), len, nullptr, nullptr);
    return c_str_sptr;
}

struct Encodings {
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756.aspx
    enum {
        ANSI = CP_ACP,
        UTF7 = CP_UTF7,
        UTF8 = CP_UTF8,
        SHIFT_JIS = 932,
        GB2312 = 936,
        KS_C_5601_1987 = 949,
        BIG5 = 950,
        JOHAB = 1361,
        EUC_JP = 51932,
        EUC_CN = 51936,
        EUC_KR = 51949,
        EUC_TW = 51950,
        GB18030 = 54936,
        GBK = GB18030,
    };
};

/**
* Encode a UTF-8 string into bytes, using the encoding specified.
*/
static bytes string_encode(const std::string &s, int encoding = Encodings::UTF8) {
    auto ws = s2ws(s);
    return bytes(widechar_to_multibyte(encoding, ws.c_str()).get());
}

/**
* Decode bytes into a UTF-8 string, using the encoding specified.
*/
static std::string string_decode(const bytes &b, int encoding = Encodings::UTF8) {
    // check if in WinNT (not Wine)
    static auto in_nt = false;

    if (!in_nt) {
        if (const auto hntdll = GetModuleHandleW(L"ntdll.dll")) {
            if (const auto pwine_get_version = GetProcAddress(hntdll, "wine_get_version");
                !pwine_get_version) {
                // has ntdll.dll but not Wine, we assume it is NT
                in_nt = true;
            }
        }
    }

    // special case for Windows NT
    if (in_nt && encoding == Encodings::ANSI && GetACP() == Encodings::GB2312) {
        // do encoding rise
        encoding = Encodings::GB18030;
    }

    auto ws = std::wstring(multibyte_to_widechar(encoding, b.c_str()).get());
    return ws2s(ws);
}
