#pragma once

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>

#include <string>

char *utf8_to_gbk_c_str(const char *utf8_str);
std::string utf8_to_gbk(const char *utf8_str);
char *gbk_to_utf8_c_str(const char *gbk_str);
std::string gbk_to_utf8(const char *gbk_str);
