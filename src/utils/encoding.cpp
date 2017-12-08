#include "./encoding.h"

#include "app.h"

#include "helpers.h"

using namespace std;

static shared_ptr<wchar_t> multibyte_to_widechar(const int code_page, const char *multibyte_str) {
    const auto len = MultiByteToWideChar(code_page, 0, multibyte_str, -1, nullptr, 0);
    auto c_wstr_sptr = make_shared_array<wchar_t>(len + 1);
    MultiByteToWideChar(code_page, 0, multibyte_str, -1, c_wstr_sptr.get(), len);
    return c_wstr_sptr;
}

static shared_ptr<char> widechar_to_multibyte(const int code_page, const wchar_t *widechar_str) {
    const auto len = WideCharToMultiByte(code_page, 0, widechar_str, -1, nullptr, 0, nullptr, nullptr);
    auto c_str_sptr = make_shared_array<char>(len + 1);
    WideCharToMultiByte(code_page, 0, widechar_str, -1, c_str_sptr.get(), len, nullptr, nullptr);
    return c_str_sptr;
}

string string_encode(const string &s, const Encoding encoding) {
    return widechar_to_multibyte(encoding, s2ws(s).c_str()).get();
}

string string_decode(const string &b, const Encoding encoding) {
    return ws2s(wstring(multibyte_to_widechar(encoding, b.c_str()).get()));
}
