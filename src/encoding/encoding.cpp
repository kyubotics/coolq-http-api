#include "encoding.h"

#include <cstdlib>

const static int ANSI = CP_ACP;
const static int UTF7 = CP_UTF7;
const static int UTF8 = CP_UTF8;
const static int SHIFT_JIS = 932;
const static int GB2312 = 936;
const static int KS_C_5601_1987 = 949;
const static int BIG5 = 950;
const static int JOHAB = 1361;
const static int EUC_JP = 51932;
const static int EUC_CN = 51936;
const static int EUC_KR = 51949;
const static int EUC_TW = 51950;
const static int GB18030 = 54936;
const static int GBK = GB18030;

wchar_t* multibyte_to_widechar(int code_page, const char* multibyte_str)
{
    auto len = MultiByteToWideChar(code_page, 0, multibyte_str, -1, nullptr, 0);
    auto wchar_str = static_cast<wchar_t *>(malloc(sizeof(wchar_t) * (len + 1)));
    memset(wchar_str, 0, sizeof(wchar_t) * (len + 1));
    MultiByteToWideChar(code_page, 0, multibyte_str, -1, wchar_str, len);
    return wchar_str;
}

char* widechar_to_multibyte(int code_page, const wchar_t* widechar_str)
{
    auto len = WideCharToMultiByte(code_page, 0, widechar_str, -1, nullptr, 0, nullptr, nullptr);
    auto multibyte_str = static_cast<char *>(malloc(sizeof(char) * (len + 1)));
    memset(multibyte_str, 0, sizeof(char) * (len + 1));
    WideCharToMultiByte(code_page, 0, widechar_str, -1, multibyte_str, len, nullptr, nullptr);
    return multibyte_str;
}

wchar_t* utf8_to_c_wstr(const char* utf8_str)
{
    return multibyte_to_widechar(UTF8, utf8_str);
}

std::wstring utf8_to_wstr(const char* utf8_str)
{
    auto c_wstr = utf8_to_c_wstr(utf8_str);
    std::wstring wstr(c_wstr);
    if (c_wstr)
        free(c_wstr);
    return wstr;
}

char* utf8_to_gbk_c_str(const char* utf8_str)
{
    auto wchar_str = multibyte_to_widechar(UTF8, utf8_str);
    auto gbk_str = widechar_to_multibyte(GB18030, wchar_str);
    if (wchar_str)
        free(wchar_str);
    return gbk_str;
}

std::string utf8_to_gbk(const char* utf8_str)
{
    auto gbk_str = utf8_to_gbk_c_str(utf8_str);
    std::string str(gbk_str);
    if (gbk_str)
        free(gbk_str);
    return str;
}

char* gbk_to_utf8_c_str(const char* gbk_str)
{
    auto wchar_str = multibyte_to_widechar(GB18030, gbk_str);
    auto utf8_str = widechar_to_multibyte(UTF8, wchar_str);
    if (wchar_str)
        free(wchar_str);
    return utf8_str;
}

std::string gbk_to_utf8(const char* gbk_str)
{
    auto utf8_str = gbk_to_utf8_c_str(gbk_str);
    std::string str(utf8_str);
    if (utf8_str)
        free(utf8_str);
    return str;
}
