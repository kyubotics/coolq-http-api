#pragma once

#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace rc {

    namespace win32 {

        static auto multibyte_to_widechar(int code_page, const char *multibyte_str) {
            auto len = MultiByteToWideChar(code_page, 0, multibyte_str, -1, nullptr, 0);
            auto c_wstr_sptr = std::shared_ptr<wchar_t>(new wchar_t[len + 1]);
            MultiByteToWideChar(code_page, 0, multibyte_str, -1, c_wstr_sptr.get(), len);
            return static_cast<std::shared_ptr<const wchar_t>>(c_wstr_sptr);
        }

        static auto widechar_to_multibyte(int code_page, const wchar_t *widechar_str) {
            auto len = WideCharToMultiByte(code_page, 0, widechar_str, -1, nullptr, 0, nullptr, nullptr);
            auto c_str_sptr = std::shared_ptr<char>(new char[len + 1]);
            WideCharToMultiByte(code_page, 0, widechar_str, -1, c_str_sptr.get(), len, nullptr, nullptr);
            return static_cast<std::shared_ptr<const char>>(c_str_sptr);
        }
    }
}
