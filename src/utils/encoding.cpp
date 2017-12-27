#include "./encoding.h"

#include "app.h"

#include <iconv.h>
#include <codecvt>

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

string ws2s(const wstring &ws) {
    return wstring_convert<codecvt_utf8<wchar_t>, wchar_t>().to_bytes(ws);
}

wstring s2ws(const string &s) {
    return wstring_convert<codecvt_utf8<wchar_t>, wchar_t>().from_bytes(s);
}

string ansi(const string &s) {
    return string_encode(s, Encodings::ANSI);
}

bytes string_encode(const string &s, const Encoding encoding) {
    return widechar_to_multibyte(encoding, s2ws(s).c_str()).get();
}

string string_decode(const bytes &b, const Encoding encoding) {
    return ws2s(wstring(multibyte_to_widechar(encoding, b.c_str()).get()));
}

static bytes iconv_convert_encoding(const bytes &text, const string &from_enc, const string &to_enc,
                                    const float capability_factor) {
    bytes result;

    const auto cd = iconv_open(to_enc.c_str(), from_enc.c_str());
    auto in = const_cast<char *>(text.data());
    auto in_bytes_left = text.size();

    if (in_bytes_left == 0) {
        return result;
    }

    auto out_bytes_left = static_cast<decltype(in_bytes_left)>(static_cast<double>(in_bytes_left) * capability_factor);
    auto out = new char[out_bytes_left] {0};
    const auto out_begin = out;

    try {
        if (static_cast<size_t>(-1) != iconv(cd, &in, &in_bytes_left, &out, &out_bytes_left)) {
            // successfully converted
            result = out_begin;
        }
    } catch (...) {}

    delete[] out_begin;
    iconv_close(cd);

    return result;
}

bytes iconv_string_encode(const string &s, const string &encoding, const float capability_factor) {
    return iconv_convert_encoding(s, "utf-8", encoding, capability_factor);
}

string iconv_string_decode(const bytes &b, const string &encoding, const float capability_factor) {
    return iconv_convert_encoding(b, encoding, "utf-8", capability_factor);
}
