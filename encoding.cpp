#include "encoding.h"

#include <cstdlib>

char *utf8_to_gbk_c_str(const char *utf8_str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0);
    wchar_t *wchar_str = (wchar_t *)malloc(sizeof(wchar_t) * (len + 1));
    memset(wchar_str, 0, sizeof(wchar_t) * (len + 1));
    MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, wchar_str, len);
    len = WideCharToMultiByte(CP_ACP, 0, wchar_str, -1, NULL, 0, NULL, NULL);
    char *gbk_str = (char *)malloc(sizeof(char) * (len + 1));
    memset(gbk_str, 0, sizeof(char) * (len + 1));
    WideCharToMultiByte(CP_ACP, 0, wchar_str, -1, gbk_str, len, NULL, NULL);
    if (wchar_str)
        free(wchar_str);
    return gbk_str;
}

std::string utf8_to_gbk(const char *utf8_str)
{
    char *gbk_str = utf8_to_gbk_c_str(utf8_str);
    std::string str(gbk_str);
    if (gbk_str)
        free(gbk_str);
    return str;
}

char *gbk_to_utf8_c_str(const char *gbk_str)
{
    int len = MultiByteToWideChar(CP_ACP, 0, gbk_str, -1, NULL, 0);
    wchar_t *wchar_str = (wchar_t *)malloc(sizeof(wchar_t) * (len + 1));
    memset(wchar_str, 0, sizeof(wchar_t) * (len + 1));
    MultiByteToWideChar(CP_ACP, 0, gbk_str, -1, wchar_str, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wchar_str, -1, NULL, 0, NULL, NULL);
    char *utf8_str = (char *)malloc(sizeof(char) * (len + 1));
    memset(utf8_str, 0, sizeof(char) * (len + 1));
    WideCharToMultiByte(CP_UTF8, 0, wchar_str, -1, utf8_str, len, NULL, NULL);
    if (wchar_str)
        free(wchar_str);
    return utf8_str;
}

std::string gbk_to_utf8(const char *gbk_str)
{
    char *utf8_str = gbk_to_utf8_c_str(gbk_str);
    std::string str(utf8_str);
    if (utf8_str)
        free(utf8_str);
    return str;
}
